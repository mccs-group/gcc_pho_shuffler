#ifndef FILE_PARSING_HH
#define FILE_PARSING_HH

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <string>
#include <sstream>
#include <utility>
#include <vector>
#include <algorithm>

#include "utilities.hh"

namespace gcc_reorder
{

// base class for parser with method to get file's content
class FileReader
{
protected:
    std::string file_content_buf;

    void get_file_text(const std::string& file_name);
};


class PassLogParser : public FileReader
{
    std::vector<pass_info> info_vec_;

public:
    // parses log with information about passes' property restrictions from the gcc itself
    void parse_log(const std::string& info_file_name);

    // parses custom constraints about reordering passes and adds the received information into existing info_vec_ vector
    std::pair<unsigned long, unsigned long> parse_constraints(const std::string& constraint_file_name, unsigned long start_custom_property);

    using iterator = std::vector<pass_info>::iterator;
    using const_iterator = std::vector<pass_info>::const_iterator;

    iterator begin() { return info_vec_.begin(); }
    iterator end() { return info_vec_.end(); }
    const_iterator begin() const { return info_vec_.begin(); }
    const_iterator end() const { return info_vec_.end(); }
    const_iterator cbegin() { return info_vec_.cbegin(); }
    const_iterator cend() { return info_vec_.cend(); }

private:
    void displace_bits(properties& prop, unsigned long displacement);
    // finds number, and return a pair of found number and iterator after position of number end
    template<typename iter>
    std::pair<unsigned long, std::string::const_iterator> find_number(iter begin, iter end)
    {
        auto&& is_digit = [](const char c){return std::isdigit(c); };
        auto&& num_begin = std::find_if(begin, end, is_digit);
        auto&& num_end = std::find_if_not(num_begin, end, is_digit);

        return {std::stoi(std::string{num_begin, num_end}), num_end};
    }


    template<typename iter>
    std::pair<std::string, properties> get_single_pass_info(iter begin, iter end)
    {
        std::pair<std::string, properties> info;
        begin = std::find_if(begin, end, [](const char c){ return isalpha(c) || (c == '*');});
        auto&& second_iter = std::find_if(begin, end, [](const char c){ return c == ' ';});

        info.first = std::string(begin, second_iter);
        if (info.first == "rtl")
            info.first.append(" pre");

        if (info.first == "end_state")
        {
            info.second = {find_number(second_iter, end).first, 0, 0};
            return info;
        }

        auto&& req_iter_pair = find_number(second_iter, end);
        auto&& prov_iter_pair = find_number(req_iter_pair.second, end);
        auto&& destr_iter_pair = find_number(prov_iter_pair.second, end);

        info.second = {req_iter_pair.first, prov_iter_pair.first, destr_iter_pair.first};

        return info;
    }

};

// reads passes to reorder from file, stores them into vector and gives access to it through begin() and end() methods
class PassToReorderParser : public FileReader
{
    std::vector<std::string> pass_vec_;

public:


    using iterator = std::vector<std::string>::iterator;
    using const_iterator = std::vector<std::string>::const_iterator;

    iterator begin() { return pass_vec_.begin(); }
    iterator end() { return pass_vec_.end(); }
    const_iterator begin() const { return pass_vec_.begin(); }
    const_iterator end() const { return pass_vec_.end(); }
    const_iterator cbegin() { return pass_vec_.cbegin(); }
    const_iterator cend() { return pass_vec_.cend(); }

    // gets passes from file to vector of strings
    void parse_passes_file(const std::string& file_name);
};



} // namespace gcc_reorder

#endif