#include "file_parsing.hh"

namespace gcc_reorder
{

void FileReader::get_file_text(const std::string& file_name)
{
    std::ifstream input;
    input.exceptions(std::ifstream::failbit);
    input.open(file_name);

    std::stringstream dump_buf;
    dump_buf << input.rdbuf();
    input.close();
    
    file_content_buf = dump_buf.str();
}

void PassLogParser::parse_log(const std::string& info_file_name)
{
    info_vec_.clear();
    try
    {
        get_file_text(info_file_name);
    }
    catch(const std::ios_base::failure& exc)
    {
        std::cerr << "Could not open file " << info_file_name << " to get passes info" << std::endl;
        throw;
    }

    auto&& second_iter = file_content_buf.cbegin();
    for (auto&& iter = file_content_buf.cbegin(); (iter != file_content_buf.cend()) && (second_iter != file_content_buf.end()); iter++)
    {
        second_iter = std::find_if(iter, file_content_buf.cend(), [](const char c){ return c == '\n';});
        auto&& info = get_single_pass_info(iter, second_iter);
        info_vec_.push_back({info.first, {info.second, {0, 0, 0}}});

        iter = second_iter;
    }

    return;
}

std::pair<unsigned long, unsigned long> PassLogParser::parse_constraints(const std::string& constraint_file_name)
{
    try
    {
        get_file_text(constraint_file_name);
    }
    catch(const std::ios_base::failure& exc)
    {
        std::cerr << "Warning! Could not open file " << constraint_file_name << " to get constraints info" << std::endl;
        return {0, 0};
    }

    if (file_content_buf.empty())
        return {0, 0};

    // skip initial comments, empty lines and spaces
    auto it = file_content_buf.cbegin();
    auto second_it = it;
    while((it = std::find(it, file_content_buf.cend(), '#')) != file_content_buf.cend())
    {
        second_it = std::find(it, file_content_buf.cend(), '\n');
        it = ++second_it;
    }
    it = second_it; // it after not finding another comment is at buf.cend, and second_it is at end of comment. So we set them both to next character

    // get initiall custom property
    auto&& it_and_start_state = find_number(it, file_content_buf.cend());
    it = ++it_and_start_state.second;


    unsigned long starting_state = it_and_start_state.first;
    unsigned long ending_state = 0;
    for (; (it != file_content_buf.cend()) && (second_it != file_content_buf.cend()); it++)
    {
        // skip comments, empty lines and spaces
        if (*it == ' ' || *it == '\n')
            continue;
        if (*it == '#')
        {
            it = std::find(it, file_content_buf.cend(), '\n');
            second_it = it;
            continue;
        }
        second_it = std::find(it, file_content_buf.cend(), '\n');

        auto&&[pass_name, prop] = get_single_pass_info(it, second_it);
        if (pass_name == "end_state")
        {
            ending_state = prop.required;
            it = std::find(it, file_content_buf.cend(), '\n');
            continue;
        }

        auto&& to_fill_constraint_it = std::find_if(info_vec_.begin(), info_vec_.end(), [&name = pass_name](const pass_info& info){ return name == info.name;});

        to_fill_constraint_it->prop.custom.required  |= prop.required;
        to_fill_constraint_it->prop.custom.provided  |= prop.provided;
        to_fill_constraint_it->prop.custom.provided  &= ~prop.destroyed;
        to_fill_constraint_it->prop.custom.destroyed |= prop.destroyed;

        it = second_it;
    }

    return {starting_state, ending_state};
}

void PassToReorderParser::parse_passes_file(const std::string& file_name)
{
    pass_vec_.clear();
    try
    {
        get_file_text(file_name);
    }
    catch(const std::ios_base::failure& exc)
    {
        std::cerr << "Could not open file " << file_name << " to get passes to reorder" << std::endl;
        throw;
    }

    auto&& second_iter = file_content_buf.begin();
    for (auto&& iter = file_content_buf.begin(); (iter != file_content_buf.end()) && (second_iter != file_content_buf.end()); iter++)
    {
        second_iter = std::find_if(iter, file_content_buf.end(), [](const char c){ return c == '\n';});
        pass_vec_.push_back({iter, second_iter});

        iter = second_iter;
    }
}

} // namespace gcc_reorder