#include "state_machine.hh"
#include "file_parsing.hh"

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

    state.num_to_prop_ = pass_to_properties_;
}

char** PassListGenerator::get_starting_action_space(int original_start_state, int custom_start_state, size_t* size_ptr)
{
    std::copy(list1.begin(), list1.end(), action_space.begin());
    std::copy(list2.begin(), list2.end(), action_space.begin() + list1.size());
    std::copy(list3.begin(), list3.end(), action_space.begin() + list1.size() + list2.size());

    *size_ptr = list1.size() + list2.size() + list3.size();

    return action_space.data();
}

char** PassListGenerator::get_action_space_helper(const char** full_action_space, int size_full, int original_state, int custom_state, size_t* size_ptr)
{
    int new_size = 0;
    for (int i = 0; i < size_full; i++)
    {
        auto&& both_props = pass_to_properties_[name_to_id_map_.at(std::string(full_action_space[i]))];

        auto&& original_required = both_props.original.required;
        auto&& custom_required = both_props.custom.required;

        if (((original_required & original_state) == original_required) && ((custom_required & custom_state) == custom_required))
        {
            std::copy(full_action_space[i], full_action_space[i] + strlen(full_action_space[i]) + 1, action_space[new_size++]);
        }
    }
    *size_ptr = new_size;

    return action_space.data();
}

char** PassListGenerator::get_new_action_space(const char** full_action_space, const char** applied_passes, int size_full,
                                               int size_applied, int original_start_state, int custom_start_state, size_t* size_ptr)
{

    if ((size_applied == 0) && (size_full == 0))
        return get_starting_action_space(original_start_state, custom_start_state, size_ptr);

    state.passes_.clear();
    state.original_property_state = original_start_state;
    state.custom_property_state = custom_start_state;
    for (int i = 0; i < size_applied; i++)
    {
        state.apply_pass(name_to_id_map_[applied_passes[i]]);
    }

    const auto original_state = state.original_property_state;
    const auto custom_state = state.custom_property_state;

    if (size_applied > 0)
    {
        if (!std::strcmp(applied_passes[size_applied - 1], "loop") || !std::strcmp(applied_passes[size_applied - 1], "loopinit"))
        {
            for (int i = 0; i < size_full; i++)
            {
                std::copy(full_action_space[i], full_action_space[i] + strlen(full_action_space[i]) + 1, swap[i]);
            }
            swapped_size = size_full;

            return get_action_space_helper(const_cast<const char**>(loop_action_space.data()), loop_action_space.size(), original_state, custom_state, size_ptr);
        }

        if (!std::strcmp(applied_passes[size_applied - 1], "loopdone"))
        {
            full_action_space = const_cast<const char**>(swap.data());
            size_full = swapped_size;
        }
    }

    return get_action_space_helper(full_action_space, size_full, original_state, custom_state, size_ptr);
}


} // namespace gcc_reorder