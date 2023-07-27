#ifndef ACTION_SPACE_HH
#define ACTION_SPACE_HH

#include "adapter.hh"
#include "file_parsing.hh"

extern "C" char** get_new_action_space(const char** full_action_space, const char** applied_passes, int size_full, 
                                       int size_applied, int list_num, size_t* size_ptr);

extern "C" int get_pass_list(char* pass_name);

extern "C" int valid_pass_seq(char** pass_seq, int size, int list_num);

extern "C" char** make_valid_pass_seq(char** pass_seq, int size, int list_num, size_t* size_ptr);

extern "C" void set_path(char* path);

#endif