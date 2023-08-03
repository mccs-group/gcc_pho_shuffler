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
        if (it->name == "loop2")
        {
            int loop2_id = 250;
            name_to_id_map_[it->name] = loop2_id;
            id_to_name[loop2_id] = it->name;
            pass_to_properties_[loop2_id] = it->prop;
        }
        else
        {
            name_to_id_map_[it->name] = i;
            id_to_name[i] = it->name;
            pass_to_properties_[i] = it->prop;
        }
    }

    state.num_to_prop_ = pass_to_properties_;
}

std::unordered_set<std::pair<unsigned long, unsigned long>> PassListGenerator::get_unique_requirements()
{
    std::unordered_set<std::pair<unsigned long, unsigned long>> unique_requirements;
    for (auto iter = info_vec_.begin(); iter != info_vec_.end(); iter++)
    {
        unique_requirements.insert({iter->prop.original.required, iter->prop.custom.required});
    }

    return unique_requirements;
}

void PassListGenerator::generate_prop_passes_map()
{
    auto&& unique_requirements = get_unique_requirements();

    for (auto&& pass_it : to_shuffle)
    {
        for (auto&& requirement_it : unique_requirements)
        {
            auto&& orig_required = pass_to_properties_.at(pass_it).original.required;
            auto&& custom_required = pass_to_properties_.at(pass_it).custom.required;
            if (std::pair{orig_required, custom_required} == requirement_it)
                unique_requirement_to_passes_[requirement_it].push_back(pass_it);
        }
    }
}

int PassListGenerator::shuffle_pass_order(const std::pair<unsigned long, unsigned long>& initial_property_state,
                                          const std::pair<unsigned long, unsigned long>& ending_property_state)
{

    // Sometimes due to properties restrictions in given range of passes some pass1 cannot be after pass2; and if pass1 is taken before pass2
    // pass2 wont be in resulting sequence at all
    // we try to avoid not using all passes in resulting sequence, so TRY_AMOUNT tries are made, so that all passes would be used
    //
    // There is a flag fail_if_not_all_passes_used, which determines, whether the generation o sequence fails alltogether, if all sequence with all
    // passes could be generated, or just the last sequence is left as resulting
    for (int i = 0; (i < TRY_AMOUNT) && (state.passes_.size() != to_shuffle.size()); i++)
    {
        // clear the previously generated sequence if there was one
        // and generate all necessary maps
        state.passes_.clear();
        unique_requirement_to_passes_.clear();
        shuffled.clear();
        generate_prop_passes_map();

        state.original_property_state = initial_property_state.first;
        state.custom_property_state = initial_property_state.second;

        std::random_device rd;
        std::mt19937 gen(rd());

        std::vector<int> passes_to_choose_from;
        passes_to_choose_from.reserve(MAX_PASS_AMOUNT); // we reserve enough space, to avoid unnecessary reallocations

        for (int i = 0; i < int(to_shuffle.size()); i++)
        {
            auto&& property_state = std::pair{state.original_property_state, state.custom_property_state};

            // fill vector of available passes
            for (auto&& it : get_unique_requirements())
            {
                // check if passes requirements are mets
                if (((property_state.first & it.first) == it.first) && ((property_state.second & it.second) == it.second) &&
                    !unique_requirement_to_passes_[it].empty())
                {
                    // copy all passes with given requirement into vector of passes to choose from
                    auto&& old_size = passes_to_choose_from.size();
                    passes_to_choose_from.resize(passes_to_choose_from.size() + unique_requirement_to_passes_[it].size());
                    std::copy(unique_requirement_to_passes_[it].begin(), unique_requirement_to_passes_[it].end(),
                            passes_to_choose_from.begin() + old_size);
                }
            }

            if (passes_to_choose_from.empty())
                break;

            // choose a pass randomly

            std::uniform_int_distribution<> to_get_index(0, passes_to_choose_from.size() - 1);

            int position_of_chosen_pass = to_get_index(gen);
            int chosen_pass = passes_to_choose_from[position_of_chosen_pass];
            state.apply_pass(chosen_pass);

            // reset available passes and erase used pass from pool of all passes to reorder
            passes_to_choose_from.clear();
            auto&& properties_of_chosen = pass_to_properties_.at(chosen_pass);

            auto&& to_erase_used_pass_from = unique_requirement_to_passes_[{properties_of_chosen.original.required, properties_of_chosen.custom.required}];
            to_erase_used_pass_from.erase(std::find(to_erase_used_pass_from.begin(), to_erase_used_pass_from.end(), chosen_pass));
        }

        // if could not meet required state by the end of sequence - regenerate
        if (((state.original_property_state & ending_property_state.first) != ending_property_state.first) ||
            ((state.custom_property_state & ending_property_state.second) != ending_property_state.second))
        {
            state.passes_.clear();
            continue;
        }
    }

    if ((state.passes_.size() != to_shuffle.size()))
        return -1; // if could not generate sequence with all passes and flag to fail in this scenario is set

    shuffled = std::move(state.passes_);

    return 0;
}

} // namespace gcc_reorder