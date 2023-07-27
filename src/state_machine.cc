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

    state.num_to_prop_ = pass_to_properties_;
}

} // namespace gcc_reorder