#ifndef STATE_MACHINE_HH
#define STATE_MACHINE_HH

#include <iostream>
#include <algorithm>
#include <numeric>
#include <random>
#include <cstring>
#include <cstdlib>
#include "utilities.hh"

namespace gcc_reorder
{

// Property state machine
//
// Gets a map: pass id -> it's properties
// Then, one by one applies passes and changes the property state correspondingly
// If a pass is met, which required properties are not met, reports a failure to std::cerr
//
// Also, has static functions to compress two set of properties/pass_properties into one
struct PropertyStateMachine
{
    std::unordered_map<int, pass_prop> num_to_prop_;
    std::vector<int> passes_;

    unsigned long original_property_state = 1; // in gcc dumps, the first pass already required an existing property 0x1
    unsigned long custom_property_state = 0;

    PropertyStateMachine() = default;

    PropertyStateMachine(const std::unordered_map<int, pass_prop>& num_to_prop) :
    num_to_prop_(num_to_prop)
    {}

    int apply_pass(int pass)
    {
        passes_.push_back(pass);
        pass_prop pass_prop = num_to_prop_.at(pass);

        if (((original_property_state & pass_prop.original.required) != pass_prop.original.required) ||
            ((custom_property_state & pass_prop.custom.required) != pass_prop.custom.required))
        {
            return -1;
        }

        custom_property_state |= pass_prop.custom.provided;
        custom_property_state &= ~pass_prop.custom.destroyed;

        original_property_state |= pass_prop.original.provided;
        original_property_state &= ~pass_prop.original.destroyed;

        return 0;
    }

};


class PassListGenerator
{
    std::vector<pass_info> info_vec_;
    std::unordered_map<std::string, int> name_to_id_map_; // we give each pass an id and work with ids to avoid needless heap indirection
    std::unordered_map<int, std::string> id_to_name;

    std::unordered_map<int, pass_prop> pass_to_properties_; // hash map: pass id -> it's properties

    PropertyStateMachine state;
    std::vector<char*> action_space;
    std::vector<char*> swap;
    std::vector<char*> loop_action_space;


public:

    static constexpr int MAX_PASS_AMOUNT = 400;
    static constexpr int MAX_PASS_LENGTH = 40;

    PassListGenerator() : action_space(MAX_PASS_AMOUNT, 0), swap(MAX_PASS_AMOUNT, 0)
    {
        std::transform(action_space.begin(), action_space.end(), action_space.begin(), [](char* ptr){ return ptr = new char[MAX_PASS_LENGTH]{0}; });
        std::transform(swap.begin(), swap.end(), swap.begin(), [](char* ptr){ return ptr = new char[MAX_PASS_LENGTH]{0}; });
    }

    ~PassListGenerator()
    {
        auto&& delete_lambda = [](char* ptr){ delete ptr; return nullptr; };
        std::transform(swap.begin(), swap.begin() + MAX_PASS_AMOUNT, swap.begin(), delete_lambda);
        std::transform(action_space.begin(), action_space.begin() + MAX_PASS_AMOUNT, action_space.begin(), delete_lambda);
        std::transform(loop_action_space.begin(), loop_action_space.end(), loop_action_space.begin(), delete_lambda);
    }

    template <typename iter_info>
    PassListGenerator(iter_info begin_info, iter_info end_info) : info_vec_{begin_info, end_info}
    {
        setup_structures();
    }

    template <typename iter>
    void set_info_vec(iter begin, iter end)
    {
        info_vec_ = {begin, end};
    }

    template <typename iter>
    void set_list4_subpasses(iter begin, iter end)
    {
        loop_action_space.resize(end - begin);

        for (int i = 0; begin != end; begin++, i++)
        {
            loop_action_space[i] = new char[MAX_PASS_LENGTH]{0};
            std::copy(begin->begin(), begin->end(), loop_action_space[i]);
        }
    }

    // map passes' names onto ids and batches of passes onto ids
    void setup_structures();

    char** get_new_action_space(const char** full_action_space, const char** applied_passes, int size_full, int size_applied,
                                int original_start_state, int custom_start_state, size_t* size_ptr);
};

} // namespace gcc_reorder

#endif