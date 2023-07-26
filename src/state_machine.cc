#include "state_machine.hh"
#include "file_parsing.hh"

#include <stack>

namespace gcc_reorder
{

// map passes' names onto ids and batches of passes onto ids
void PassListGenerator::setup_structures()
{
    int i = 0;

    for (auto&& it = info_vec_.begin(); it != info_vec_.end(); it++, i++)
    {
        name_to_id_map_[it->name] = i;
        id_to_name[i] = it->name;
        pass_to_properties_[i] = it->prop;
    }

    // std::cerr << pass_to_properties_[name_to_id_map_["widening_mul"]].custom.required << std::endl;


    if (!all_lists.empty())
        return;

    all_lists.reserve(list1.size() + list2.size() + list3.size());

    for (auto&& it : list1)
    {
        // std::cout << std::string{it} << std::endl;
        pass_to_list_num[it] = 1;
        all_lists.push_back(it);
    }

    for (auto&& it : list2)
    {
        // std::cout << std::string{it} << std::endl;
        pass_to_list_num[it] = 2;
        all_lists.push_back(it);
    }

    for (auto&& it : list3)
    {
        // std::cout << std::string{it} << std::endl;
        pass_to_list_num[it] = 3;
        all_lists.push_back(it);
    }

    for (auto&& it : loop_action_space)
    {
        // std::cout << std::string{it} << std::endl;
        pass_to_list_num[it] = 2;
    }
}

void PassListGenerator::change_list(const std::vector<std::pair<unsigned long, unsigned long>>& start_end_prop)
{
    state.num_to_prop_ = pass_to_properties_;

    for (auto&& it : info_vec_)
    {
        if (pass_to_list_num.find(it.name) != pass_to_list_num.end())
            for (int j = 0; j < pass_to_list_num.at(it.name) - 1; j++)
            {
                // std::cout << "Adding to " << it.name << ' ' << start_end_prop[j].second <<  std::endl;
                it.prop.custom.required |= start_end_prop[j].second;
            }
    }

    for (auto&& it : info_vec_)
    {
        pass_to_properties_[name_to_id_map_.at(it.name)] = it.prop;
    }

    for (int j = 0; j < 3; j++)
    {
        start_properties[j + 1].second = start_end_prop[j].first;
        end_properties[j + 1].second = start_end_prop[j].second;
        start_properties[0].second |= start_properties[j + 1].second;
    }
    end_properties[0].second = end_properties[3].second;

}

int PassListGenerator::valid_pass_seq(char** pass_seq, int size, int list_num)
{
    state.original_property_state = start_properties[list_num].first;
    state.custom_property_state = start_properties[list_num].second;

    char** bad = std::find_if(pass_seq, pass_seq + size, [this](char* str){ return state.apply_pass(name_to_id_map_[std::string{str}]) != 0;});

    if (bad != pass_seq + size)
    {
        return bad - pass_seq + 1;
    }

    if ((state.custom_property_state & end_properties[list_num].second) != end_properties[list_num].second)
        return size;

    return 0;
}

char** PassListGenerator::make_valid_pass_seq(char** pass_seq, int size, int list_num, size_t* size_ptr)
{
    state.original_property_state = start_properties[list_num].first;
    state.custom_property_state = start_properties[list_num].second;

    for (int i = 0; i < size; i++)
    {
        std::copy(pass_seq[i], pass_seq[i] + strlen(pass_seq[i]) + 1, action_space[i]);
        state.apply_pass(name_to_id_map_[pass_seq[i]]);
    }

    std::stack<const char*> passes;

    auto ending_state_diff = end_properties[list_num].second & (~state.custom_property_state);
    auto&& necessary_pass_finder = [&ending_state_diff](const pass_info& info){return (info.prop.custom.provided & ending_state_diff) == ending_state_diff && 
                                                                                        (info.prop.custom.destroyed & ending_state_diff) != ending_state_diff;};

    while (ending_state_diff != 0)
    {
        auto&& pass_info_it = std::find_if(info_vec_.begin(), info_vec_.end(), necessary_pass_finder);
        passes.push(pass_info_it->name.c_str());

        ending_state_diff = pass_info_it->prop.custom.required & (~state.custom_property_state);
    }

    *size_ptr = size + passes.size();

    for (int i = 0; !passes.empty(); i++)
    {
        auto&& to_copy_from = passes.top();
        std::copy(to_copy_from, to_copy_from + strlen(to_copy_from) + 1, action_space[size + i]);
        passes.pop();
    }

    return action_space.data();
}

char** PassListGenerator::get_new_action_space(const char** full_action_space, const char** applied_passes, int size_full,
                                               int size_applied, int list_num, size_t* size_ptr)
{

    state.passes_.clear();
    state.original_property_state = start_properties[list_num].first;
    state.custom_property_state = start_properties[list_num].second;

    for (int i = 0; i < size_applied; i++)
    {
        state.apply_pass(name_to_id_map_[applied_passes[i]]);
    }

    const auto original_state = state.original_property_state;
    const auto custom_state = state.custom_property_state;

    if (size_applied > 0)
    {
        if (!std::strcmp(applied_passes[size_applied - 1], "fix_loops"))
        {
            const char* loop = "loop";
            std::copy(loop, loop + strlen(loop) + 1, action_space[0]);
            *size_ptr = 1;
            return action_space.data();
        }
        if (!std::strcmp(applied_passes[size_applied - 1], "loop"))
        {
            const char* loopinit = "loopinit";
            std::copy(loopinit, loopinit + strlen(loopinit) + 1, action_space[0]);
            *size_ptr = 1;
            return action_space.data();
        }

        if (!std::strcmp(applied_passes[size_applied - 1], "loopinit"))
            in_loop = true;

        if (!std::strcmp(applied_passes[size_applied - 1], "loopdone"))
            in_loop = false;
    }

    if(in_loop)
        return get_action_space_helper(loop_action_space.begin(), loop_action_space.end(), original_state, custom_state, size_ptr);

    switch (list_num)
    {
        case 1:
                return get_action_space_helper(list1.begin(), list1.end(), original_state, custom_state, size_ptr);
                break;
        case 2:
                return get_action_space_helper(list2.begin(), list2.end(), original_state, custom_state, size_ptr);
                break;
        case 3:
                return get_action_space_helper(list3.begin(), list3.end(), original_state, custom_state, size_ptr);
                break;
        case 0: return get_action_space_helper(all_lists.begin(), all_lists.end(), original_state, custom_state, size_ptr);
                break;
        default:
                std::cerr << "Unknown list num " << list_num << std::endl;
                *size_ptr = 0;
                return nullptr;
    }
}

int PassListGenerator::get_pass_list(char* pass_name)
{
    if (pass_to_list_num.find(pass_name) == pass_to_list_num.end())
        return -1;

    return pass_to_list_num.at(std::string{pass_name});
}

} // namespace gcc_reorder