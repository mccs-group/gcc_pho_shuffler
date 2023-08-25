#include "action_space.hh"

gcc_reorder::PassListGenAdapter adapter{};

extern "C" char** get_new_action_space(const char** full_action_space,const char** applied_passes, int size_full,
                                       int size_applied, int list_num, size_t* size_ptr)
{
    return adapter.get_new_action_space(applied_passes, size_applied, list_num, size_ptr);
}

extern "C" int get_pass_list(char* pass_name)
{
    return adapter.get_pass_list(pass_name);
}

extern "C" int valid_pass_seq(char** pass_seq, int size, int list_num)
{
    return adapter.valid_pass_seq(pass_seq, size, list_num);
}

extern "C" char** make_valid_pass_seq(char** pass_seq, int size, int list_num, size_t* size_ptr)
{
    return adapter.make_valid_pass_seq(pass_seq, size, list_num, size_ptr);

}

extern "C" int* get_shuffled_list(int list_num, size_t* size_ptr)
{
    return adapter.get_shuffled_list(list_num, size_ptr);
}

char** get_action_space_by_property(unsigned long orig_prop_state, unsigned long custom_prop_state, int list_num, size_t* size_ptr)
{
    return adapter.get_action_space_by_property({orig_prop_state, custom_prop_state}, list_num, size_ptr);
}

char** get_list_by_list_num(int list_num, size_t* size_ptr)
{
    return adapter.get_list_by_list_num(list_num, size_ptr);
}

void get_property_by_history(char** pass_seq, int size, int list_num, size_t* orig_ptr, size_t* custom_ptr)
{
    auto prop_pair = adapter.get_property_by_history(pass_seq, size, list_num);
    *orig_ptr = prop_pair.first;
    *custom_ptr = prop_pair.second;
}


extern "C" void set_include_used(int flag)
{
    if (flag == 0)
        adapter.set_include_used(false);
    else
        adapter.set_include_used(true);
}

extern "C" void set_path(char* path)
{
    adapter.set_path_to_dir(std::filesystem::path{path}.parent_path());
    adapter.setup();
}


int if_in_loop(size_t custom_prop)
{
    return adapter.if_in_loop(custom_prop);
}

