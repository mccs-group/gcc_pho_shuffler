#ifndef UTILITIES_HH
#define UTILITIES_HH

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <string>
#include <sstream>
#include <utility>
#include <vector>
#include <algorithm>

namespace gcc_reorder
{

struct properties
{
    unsigned long required = 0;
    unsigned long provided = 0;
    unsigned long destroyed = 0;
};

inline bool operator==(const properties& lhs, const properties& rhs)
{
    return (lhs.required == rhs.required) && 
           (lhs.provided == rhs.provided) && 
           (lhs.destroyed == rhs.destroyed);
}

struct pass_prop
{
    properties original; // the once dumped with the help of plugin from gcc
    properties custom;   // custom ones, needed to restrict reordering if some hidden dependencies are found
};

inline bool operator==(const pass_prop& lhs, const pass_prop& rhs)
{
    return (lhs.original == rhs.original) && (lhs.custom == rhs.custom);
}

struct pass_info
{
    std::string name;
    pass_prop prop;
};

inline bool operator==(const pass_info& lhs, const pass_info& rhs)
{
    return (lhs.name == rhs.name) && (lhs.prop == rhs.prop);
}

} // namespace gcc_reorder

namespace std // necessary specializations of std::hash to use std::unordered_...<pass_info> and std::unordered_...<std::pair<ulong, ulong>>
{
    template<>
    struct hash<gcc_reorder::pass_info>
    {
        typedef gcc_reorder::pass_info argument_type;
        std::size_t operator()(const argument_type& info) const
        {
            std::size_t h1 = std::hash<std::string>{}(info.name);
            std::size_t h2 = std::hash<unsigned long>{}(info.prop.custom.required + info.prop.original.required) << 1;
            std::size_t h3 = std::hash<unsigned long>{}(info.prop.custom.required + info.prop.original.required) << 2;
            std::size_t h4 = std::hash<unsigned long>{}(info.prop.custom.required + info.prop.original.required) << 3;
            return h1 ^ h2 ^ h3 ^ h4;
        }
    };

    template<>
    struct hash<std::pair<unsigned long, unsigned long>>
    {
        typedef std::pair<unsigned long, unsigned long> argument_type;
        std::size_t operator()(const argument_type& pair) const
        {
            std::size_t h1 = std::hash<unsigned long>{}(pair.first);
            std::size_t h2 = std::hash<unsigned long>{}(pair.second) << 1;
            return h1 ^ h2;
        }
    };
}

#endif