#ifndef ADAPTER_HH
#define ADAPTER_HH

#include "state_machine.hh"
#include "file_parsing.hh"

namespace gcc_reorder
{

class PassListGenAdapter
{
    std::unordered_map<std::string, int> pass_to_list_num;
    static constexpr std::array<unsigned long, 4> start_original_properties = { 76079 | 130760, 76079, 76079, 130760};
    std::pair<unsigned long, unsigned long> custom_properties = {0, 0};

    std::vector<std::string> list1;
    std::vector<std::string> list2;
    std::vector<std::string> list3;
    std::vector<std::string> loop_action_space;
    std::vector<std::string> all_lists;

    std::vector<std::vector<char>> buf;
    std::vector<char*> buf_to_return;

    int current_list_num = -1;
    int starting_list_num = -1;
    PassListGenerator gen;
    PassLogParser log_parser;

    bool in_loop = false;

public:
    PassListGenAdapter();


    char** get_new_action_space(const char** applied_passes, int size_applied, int list_num, size_t* size_ptr);

    int get_pass_list(char* pass_name);

    int valid_pass_seq(char** pass_seq, int size, int list_num);

    char** make_valid_pass_seq(char** pass_seq, int size, int list_num, size_t* size_ptr);

    static constexpr int MAX_PASS_AMOUNT = 400;
    static constexpr int MAX_PASS_LENGTH = 40;
private:

    void change_current_list_num(int list_num);
};

}

#endif