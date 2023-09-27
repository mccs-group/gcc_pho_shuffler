#ifndef ACTION_SPACE_HH
#define ACTION_SPACE_HH

#include "adapter.hh"
#include "file_parsing.hh"

extern "C" char** get_new_action_space(const char** full_action_space, const char** applied_passes, int size_full, 
                                       int size_applied, int list_num, size_t* size_ptr);

extern "C" int get_pass_list(char* pass_name);

extern "C" int valid_pass_seq(char** pass_seq, int size, int list_num);

extern "C" char** make_valid_pass_seq(char** pass_seq, int size, int list_num, size_t* size_ptr);

extern "C" void set_include_used(int flag);

extern "C" void set_path(char* path);

extern "C" int* get_shuffled_list(int list_num, size_t* size_ptr);

extern "C" char** get_action_space_by_property(unsigned long orig_prop_state, unsigned long custom_prop_state, int list_num, size_t* size_ptr);

extern "C" char** get_list_by_list_num(int list_num, size_t* size_ptr);

extern "C" void get_property_by_history(char** pass_seq, int size, int list_num, size_t* orig_ptr, size_t* custom_ptr);

extern "C" int if_in_loop(size_t custom_prop);

extern "C" void set_check_loop(int flag);


#endif