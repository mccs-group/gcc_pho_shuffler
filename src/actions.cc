#include "state_machine.hh"
#include "file_parsing.hh"

extern "C" char** get_new_action_space(const char** full_action_space,const char** applied_passes, int size_full, int size_applied, int original_start_state, int custom_start_state,
                                       size_t* size_ptr)
{
    static bool initialized = false;
    static gcc_reorder::PassListGenerator gen;

    if (!initialized)
    {
        gcc_reorder::PassLogParser log_parser;
        gcc_reorder::PassToReorderParser pass_parser; 
        log_parser.parse_log("unique_passes.txt");
        gen.set_info_vec(log_parser.begin(), log_parser.end());

        pass_parser.parse_passes_file("lists/to_shuffle4.txt");
        gen.set_list4_subpasses(pass_parser.begin(), pass_parser.end());
        gen.setup_structures();

        // for (auto&& it : gen.info_vec_)
        //     std::cout << it.name << ' ' << it.prop.original.required << ' ' << it.prop.original.provided << ' ' << it.prop.original.destroyed <<
        //     ' ' << it.prop.custom.required << ' ' << it.prop.custom.provided << ' ' << it.prop.custom.destroyed << std::endl;
        initialized = true;
    }

    return gen.get_new_action_space(full_action_space, applied_passes, size_full, size_applied, original_start_state, custom_start_state, size_ptr);
}