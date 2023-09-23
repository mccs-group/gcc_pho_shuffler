#ifndef ADAPTER_HH
#define ADAPTER_HH

#include <filesystem>

#include "state_machine.hh"
#include "file_parsing.hh"

#include <array>

namespace gcc_reorder
{

class PassListGenAdapter
{
    std::unordered_map<std::string, int> pass_to_list_num;
    static constexpr std::array<unsigned long, 5> start_original_properties = { 76079 | 130760, 76079, 76079, 130760, 126255};
    std::pair<unsigned long, unsigned long> custom_properties = {0, 0};

    static constexpr unsigned long IN_LOOP_SECOND_LIST_PROP = 8;
    static constexpr unsigned long FIX_LOOP_INDICATOR = 32;
    static constexpr unsigned long PRE_INDICATOR = 2;

    std::vector<std::string> list1;
    std::vector<std::string> list2;
    std::vector<std::string> list3;
    std::vector<std::string> loop_action_space;
    std::vector<std::string> all_lists;

    std::filesystem::path path_to_dir;

    std::vector<std::vector<char>> buf;
    std::vector<char*> buf_to_return;

    std::vector<int> shuffled_passes_id;

    int current_list_num = -1;
    int starting_list_num = -1;
    PassListGenerator gen;
    PassLogParser log_parser;

    bool in_loop = false;

public:
    void setup();

    void set_path_to_dir(const std::filesystem::path& path) { path_to_dir = path;}

    char** get_new_action_space(const char** applied_passes, int size_applied, int list_num, size_t* size_ptr);

    int* get_shuffled_list(int list_num, size_t* size_ptr);

    int get_pass_list(char* pass_name);

    int valid_pass_seq(char** pass_seq, int size, int list_num);

    char** make_valid_pass_seq(char** pass_seq, int size, int list_num, size_t* size_ptr);

    void set_include_used(bool flag) {gen.include_used = flag;}

    char** get_action_space_by_property(std::pair<unsigned long, unsigned long> property_state, int list_num, size_t* size_ptr);

    char** get_list_by_list_num(int list_num, size_t* size_ptr);

    int if_in_loop(unsigned long custom_prop);

    std::pair<unsigned long, unsigned long> get_property_by_history(char** pass_seq, int size, int list_num);


    static constexpr int MAX_PASS_AMOUNT = 400;
    static constexpr int MAX_PASS_LENGTH = 40;
private:

    template <typename iter>
    int verify_sub_loops(iter begin, iter end)
    {
        std::vector<int> passes(loop_action_space.size(), 0);
        gen.map_names_onto_id(loop_action_space.begin(), loop_action_space.end(), passes.begin());

        auto loop_subpass = [&pass_vec = passes](int pass_id){ return std::find(pass_vec.begin(), pass_vec.end(), pass_id) != pass_vec.end(); };

        auto non_loop_subpass_it = std::find_if_not(begin, end, loop_subpass);

        if (non_loop_subpass_it != end)
            return std::distance(begin, non_loop_subpass_it) + 1;

        return 0;
    }

    template <typename iter>
    void set_pass_names_vec(iter begin, iter end)
    {
        int new_size = 0;
        std::vector<std::string> new_action_space;
        gen.map_id_onto_names(begin, end, std::back_inserter(new_action_space));
        for (auto&& it : new_action_space)
        {
            std::copy(it.c_str(), it.c_str() + it.size() + 1, buf[new_size].data());
            buf_to_return.push_back(buf[new_size].data());
            new_size++;
            // std::cout << it << std::endl;
        }
    }

    void change_current_list_num(int list_num);

    template <typename it>
    void set_start_list(int list_num, it output)
    {
        switch (list_num)
        {
            case 1:
                    gen.map_names_onto_id(list1.begin(), list1.end(), output);
                    break;
            case 2:
                    gen.map_names_onto_id(list2.begin(), list2.end(), output);
                    break;
            case 3:
                    gen.map_names_onto_id(list3.begin(), list3.end(), output);
                    break;
            case 4:
                    gen.map_names_onto_id(loop_action_space.begin(), loop_action_space.end(), output);
                    break;
            case 0:
                    gen.map_names_onto_id(all_lists.begin(), all_lists.end(), output);
                    break;
            default:
                    std::cerr << "Unknown list num " << list_num << std::endl;
                    return;
        }
    }
};

}

#endif
