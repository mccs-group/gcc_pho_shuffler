#include "adapter.hh"

namespace gcc_reorder
{

void PassListGenAdapter::setup()
{
    PassToReorderParser pass_parser;

    pass_parser.parse_passes_file(path_to_dir / "lists/to_shuffle1.txt");
    list1 = {pass_parser.begin(), pass_parser.end()};

    pass_parser.parse_passes_file(path_to_dir / "lists/to_shuffle2.txt");
    list2 = {pass_parser.begin(), pass_parser.end()};

    pass_parser.parse_passes_file(path_to_dir / "lists/to_shuffle3.txt");
    list3 = {pass_parser.begin(), pass_parser.end()};

    pass_parser.parse_passes_file(path_to_dir / "lists/to_shuffle4.txt");
    loop_action_space = {pass_parser.begin(), pass_parser.end()};

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

    buf.resize(MAX_PASS_AMOUNT);
    std::for_each(buf.begin(), buf.end(), [](std::vector<char>& vec){vec.resize(MAX_PASS_LENGTH, 0);});
}

int* PassListGenAdapter::get_shuffled_list(int list_num, size_t* size_ptr)
{
    if (list_num != current_list_num)
        change_current_list_num(list_num);

    std::vector<int> passes_id;

    const std::vector<std::string> loop2_subpasses = {"loop2_init", "loop2_invariant", "loop2_unroll", "loop2_doloop", "loop2_done"};
    std::vector<std::string> list3_without_loop2;

    auto&& not_loop2_subpasses = [begin = loop2_subpasses.begin(), end = loop2_subpasses.end()]
                                    (const std::string& str){return std::find(begin, end, str) == end;};

    std::vector<std::string> list2_without_loop;
    auto&& not_loop = [](const std::string& str){return str != "loop";};

    switch (list_num)
    {
        case 1:
                passes_id.resize(list1.size());
                gen.map_onto_id(list1.begin(), list1.end(), passes_id.begin());
                break;
        case 2:
                std::copy_if(list2.begin(), list2.end(), std::back_inserter(list2_without_loop), not_loop);
                passes_id.resize(list2_without_loop.size());
                gen.map_onto_id(list2_without_loop.begin(), list2_without_loop.end(), passes_id.begin());
                break;
        case 3:

                std::copy_if(list3.begin(), list3.end(), std::back_inserter(list3_without_loop2), not_loop2_subpasses);
                passes_id.resize(list3_without_loop2.size());
                gen.map_onto_id(list3_without_loop2.begin(), list3_without_loop2.end(), passes_id.begin());
                break;
        case 4:
                passes_id.resize(list3.size());
                gen.map_onto_id(list3.begin(), list3.end(), passes_id.begin());
                break;
        case 0:
                passes_id.resize(all_lists.size());
                gen.map_onto_id(all_lists.begin(), all_lists.end(), passes_id.begin());
                break;
        default:
                std::cerr << "Unknown list num " << list_num << std::endl;
                *size_ptr = 0;
                return nullptr;
    }

    gen.to_shuffle = std::move(passes_id);
    int result = gen.shuffle_pass_order({start_original_properties[list_num], custom_properties.first}, {0, custom_properties.second});

    if (result == -1)
    {
        *size_ptr = 0;
        return nullptr;
    }

    shuffled_passes_id = std::move(gen.shuffled);

    *size_ptr = shuffled_passes_id.size();

    return shuffled_passes_id.data();
}

void PassListGenAdapter::change_current_list_num(int list_num)
{
    log_parser.parse_log(path_to_dir / "unique_passes.txt");
    std::vector constraints_vec = {path_to_dir / "lists/constraints1.txt", path_to_dir / "lists/constraints2.txt",
                                   path_to_dir / "lists/constraints3.txt", path_to_dir / "lists/constraints4.txt"};

    custom_properties = {0, 0};
    std::vector<unsigned long> ending_prop(4, 0);

    if (list_num == 0)
    {
        unsigned long custom_ending_prop = 0;
        std::pair<unsigned long, unsigned long> current_list_custom_prop{0, 0};
        int i = 0;
        for (auto&& it = constraints_vec.begin(); it != constraints_vec.end(); it++, i++)
        {
            current_list_custom_prop = log_parser.parse_constraints(*it, custom_ending_prop);

            custom_ending_prop |= current_list_custom_prop.second;
            ending_prop[i] = current_list_custom_prop.second;
            custom_properties.first |= current_list_custom_prop.first;
        }
        custom_properties.second = current_list_custom_prop.second;
    }
    else
    {
        custom_properties = log_parser.parse_constraints(constraints_vec[list_num - 1], 0);
    }

    if (list_num != 4)
        for (auto&& it : log_parser)
        {
            if (pass_to_list_num.find(it.name) != pass_to_list_num.end())
                for (int j = 0; j < pass_to_list_num.at(it.name) - 1; j++)
                {
                    // std::cout << "Adding to " << it.name << ' ' << start_end_prop[j].second <<  std::endl;
                    it.prop.custom.required |= ending_prop[j];
                }
        }

    gen.set_info_vec(log_parser.begin(), log_parser.end());
    gen.setup_structures();

    if (starting_list_num == -1)
        starting_list_num = list_num;

    current_list_num = list_num;
}

char** PassListGenAdapter::get_new_action_space(const char** applied_passes, int size_applied, int list_num, size_t* size_ptr)
{
    if (list_num != current_list_num && !((list_num == 2) && (current_list_num == 4)))
        change_current_list_num(list_num);

    buf_to_return.clear();
    // for (auto&& it : gen.info_vec_)
    // {
    //     std::cout << it.name << ' ' << it.prop.original.required << ' ' << it.prop.original.provided << ' ' << it.prop.original.destroyed
    //     << ' ' << it.prop.custom.required << ' ' << it.prop.custom.provided << ' ' << it.prop.custom.destroyed << std::endl;
    // }

    if (size_applied > 0)
    {
        if (!std::strcmp(applied_passes[size_applied - 1], "fix_loops"))
        {
            const char* loop = "loop";
            std::copy(loop, loop + strlen(loop) + 1, buf[0].data());
            buf_to_return[0] = buf[0].data();
            *size_ptr = 1;
            return buf_to_return.data();
        }
        if (!std::strcmp(applied_passes[size_applied - 1], "loop"))
        {
            const char* loopinit = "loopinit";
            std::copy(loopinit, loopinit + strlen(loopinit) + 1, buf[0].data());
            buf_to_return[0] = buf[0].data();
            *size_ptr = 1;
            return buf_to_return.data();
        }

        if (!std::strcmp(applied_passes[size_applied - 1], "loopinit"))
        {
            change_current_list_num(4);
            in_loop = true;
        }

        if (!std::strcmp(applied_passes[size_applied - 1], "loopdone"))
        {
            change_current_list_num(2);
            in_loop = false;
        }
    }

    std::vector<int> action_space_ids;
    if (in_loop)
    {
        action_space_ids.resize(loop_action_space.size());
        gen.map_onto_id(loop_action_space.begin(), loop_action_space.end(), action_space_ids.begin());
    }
    else
    {
        switch (list_num)
        {
            case 1:
                    action_space_ids.resize(list1.size());
                    gen.map_onto_id(list1.begin(), list1.end(), action_space_ids.begin());
                    break;
            case 2:
                    action_space_ids.resize(list2.size());
                    gen.map_onto_id(list2.begin(), list2.end(), action_space_ids.begin());
                    break;
            case 3:
                    action_space_ids.resize(list3.size());
                    gen.map_onto_id(list3.begin(), list3.end(), action_space_ids.begin());
                    break;
            case 0:
                    action_space_ids.resize(all_lists.size());
                    gen.map_onto_id(all_lists.begin(), all_lists.end(), action_space_ids.begin());
                    break;
            default:
                    std::cerr << "Unknown list num " << list_num << std::endl;
                    *size_ptr = 0;
                    return nullptr;
        }

    }


    gen.set_full_action_space_vec(action_space_ids.begin(), action_space_ids.end());

    std::vector<int> applied_passes_ids;
    applied_passes_ids.resize(size_applied);
    gen.map_onto_id(applied_passes, applied_passes + size_applied, applied_passes_ids.begin());

    // for (auto&& it : applied_passes_ids)
    //     std::cout << it << std::endl;

    gen.get_new_action_space(applied_passes_ids.begin(), applied_passes_ids.end(), {start_original_properties[list_num], custom_properties.first});

    int new_size = 0;
    for (auto&& it : gen)
    {
        std::copy(it.c_str(), it.c_str() + it.size() + 1, buf[new_size].data());
        buf_to_return.push_back(buf[new_size].data());
        new_size++;
        // std::cout << it << std::endl;
    }
    *size_ptr = buf_to_return.size();

    return buf_to_return.data();
}

int PassListGenAdapter::get_pass_list(char* pass_name)
{
    if (pass_to_list_num.find(pass_name) == pass_to_list_num.end())
        return -1;

    return pass_to_list_num.at(std::string{pass_name});
}


int PassListGenAdapter::valid_pass_seq(char** pass_seq, int size, int list_num)
{
    if (list_num != current_list_num)
        change_current_list_num(list_num);

    std::vector<int> passes(size, 0);
    gen.map_onto_id(pass_seq, pass_seq + size, passes.begin());

    if(list_num == 3)
        if (int loop2_invalid = gen.check_loop2(passes.begin(), passes.end()); loop2_invalid != 0)
            return loop2_invalid;

    return gen.valid_pass_seq(passes.begin(), passes.end(), {start_original_properties[list_num], custom_properties.first}, custom_properties.second);
}

char** PassListGenAdapter::make_valid_pass_seq(char** pass_seq, int size, int list_num, size_t* size_ptr)
{
    if (list_num != current_list_num)
        change_current_list_num(list_num);

    buf_to_return.clear();
    std::vector<int> current_seq(size, 0);
    gen.map_onto_id(pass_seq, pass_seq + size, current_seq.begin());

    gen.make_valid_pass_seq(current_seq.begin(), current_seq.end(), {start_original_properties[list_num], custom_properties.first}, custom_properties.second);

    for (auto&& it : gen)
    {
        buf_to_return.push_back(const_cast<char*>(it.c_str()));
        // std::cout << it << std::endl;
    }
    *size_ptr = buf_to_return.size();

    return buf_to_return.data();
}

}