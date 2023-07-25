#include "state_machine.hh"
#include "file_parsing.hh"

namespace gcc_reorder
{

// map passes' names onto ids and batches of passes onto ids
void PassListGenerator::setup_structures(const std::vector<std::pair<unsigned long, unsigned long>> & vec)
{
    int i = 0;

    for (auto&& it = info_vec_.begin(); it != info_vec_.end(); it++, i++)
    {
        name_to_id_map_[it->name] = i;
        id_to_name[i] = it->name;
        pass_to_properties_[i] = it->prop;
    }

    state.num_to_prop_ = pass_to_properties_;

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

    add_list_ordering(vec);
}

void PassListGenerator::add_list_ordering(const std::vector<std::pair<unsigned long, unsigned long>>& vec)
{
    for (auto&& it : info_vec_)
    {
        if (pass_to_list_num.find(it.name) != pass_to_list_num.end())
            it.prop.custom.required |= vec[pass_to_list_num.at(it.name) - 1].second;
    }

    start_properties[1].second = vec[0].first;

    start_properties[2].second = vec[1].first | vec[0].second;
    start_properties[3].second = vec[2].first | vec[0].second | vec[1].second;

    for (int i = 1; i < 4; i++)
        start_properties[0].second |= start_properties[i].second;

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
    else
        return 0;
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

    // std::cout << "state" << std::endl;
    // std::cout << original_state << ' ' << custom_state << std::endl;

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
    return pass_to_list_num.at(std::string{pass_name});
}

} // namespace gcc_reorder