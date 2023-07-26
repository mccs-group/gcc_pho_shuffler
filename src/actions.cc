#include "action_space.hh"

gcc_reorder::PassListGenerator gen;


void set_info_vec(int list_num)
{
    gcc_reorder::PassLogParser log_parser;
    log_parser.parse_log("../shuffler/unique_passes.txt");
    std::vector constraints_vec = {"../shuffler/lists/constraints1.txt", "../shuffler/lists/constraints2.txt", "../shuffler/lists/constraints3.txt"};
    std::vector<std::pair<unsigned long, unsigned long>> start_prop(3, {0, 0});

    unsigned long custom_ending_constraint = 0;
    if (list_num == 0)
    {
        int i = 0;
        for (auto&& it = constraints_vec.begin(); it != constraints_vec.end(); it++, i++)
        {
            start_prop[i] = log_parser.parse_constraints(*it, custom_ending_constraint);
            custom_ending_constraint |= start_prop[i].second;
        }
    }
    else
    {
        start_prop[list_num - 1] = log_parser.parse_constraints(constraints_vec[list_num - 1], 0);
    }

    // std::cout << "in log" << std::endl;

    // for (auto&& it : log_parser)
    // {
    //     std::cout << it.name << ' ' << it.prop.original.required << ' ' << it.prop.original.provided << ' ' << it.prop.original.destroyed
    //     << ' ' << it.prop.custom.required << ' ' << it.prop.custom.provided << ' ' << it.prop.custom.destroyed << std::endl;
    // }

    gen.set_info_vec(log_parser.begin(), log_parser.end());

    // for (auto&& it : start_prop)
    // {
    //     std::cout << it.first << ' ' << it.second << std::endl;
    // }

    gen.setup_structures();
    gen.change_list(start_prop);
    gen.current_list = list_num;

}

void init(int list_num)
{
    gcc_reorder::PassToReorderParser pass_parser;

    pass_parser.parse_passes_file("../shuffler/lists/to_shuffle1.txt");
    gen.set_list1(pass_parser.begin(), pass_parser.end());

    pass_parser.parse_passes_file("../shuffler/lists/to_shuffle2.txt");
    gen.set_list2(pass_parser.begin(), pass_parser.end());

    pass_parser.parse_passes_file("../shuffler/lists/to_shuffle3.txt");
    gen.set_list3(pass_parser.begin(), pass_parser.end());

    pass_parser.parse_passes_file("../shuffler/lists/to_shuffle4.txt");
    gen.set_list4_subpasses(pass_parser.begin(), pass_parser.end());

    set_info_vec(list_num);

    // std::cout << "in gen" << std::endl;

    // for (auto&& it : gen.info_vec_)
    // {
    //     std::cout << it.name << ' ' << it.prop.original.required << ' ' << it.prop.original.provided << ' ' << it.prop.original.destroyed
    //     << ' ' << it.prop.custom.required << ' ' << it.prop.custom.provided << ' ' << it.prop.custom.destroyed << std::endl;
    // }
    gen.starting_list = list_num;
}


extern "C" char** get_new_action_space(const char** full_action_space,const char** applied_passes, int size_full,
                                       int size_applied, int list_num, size_t* size_ptr)
{
    static bool initialized = false;

    if (!initialized)
    {
        init(list_num);
        initialized = true;
    }

    if(gen.current_list != list_num)
    {
        set_info_vec(list_num);
    }

    return gen.get_new_action_space(full_action_space, applied_passes, size_full, size_applied, list_num, size_ptr);
}

extern "C" int get_pass_list(char* pass_name)
{
    static bool initialized = false;

    if (!initialized)
    {
        init(0);
        initialized = true;
    }

    return gen.get_pass_list(pass_name);
}

extern "C" int valid_pass_seq(char** pass_seq, int size, int list_num)
{
    static bool initialized = false;

    if (!initialized)
    {
        init(list_num);
        initialized = true;
    }

    if(gen.current_list != list_num)
    {
        set_info_vec(list_num);
    }

    return gen.valid_pass_seq(pass_seq, size, list_num);
}

extern "C" char** make_valid_pass_seq(char** pass_seq, int size, int list_num)
{
    static bool initialized = false;

    if (!initialized)
    {
        init(list_num);
        initialized = true;
    }

    return nullptr;

}
