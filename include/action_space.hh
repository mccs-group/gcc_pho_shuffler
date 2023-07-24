#ifndef ACTION_SPACE_HH
#define ACTION_SPACE_HH

#include "state_machine.hh"
#include "file_parsing.hh"

extern "C" char** get_new_action_space(const char** full_action_space,const char** applied_passes, int size_full, 
                                       int size_applied, int original_start_state, int custom_start_state, size_t* size_ptr);

#endif