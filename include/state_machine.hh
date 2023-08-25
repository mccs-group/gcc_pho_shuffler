#ifndef STATE_MACHINE_HH
#define STATE_MACHINE_HH

#include <iostream>
#include <algorithm>
#include <numeric>
#include <random>
#include <cstring>
#include <cstdlib>
#include "utilities.hh"
#include <stack>

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

    static constexpr int EMPTY_PASS = -10;

    bool can_be_applied(int pass)
    {
        if (pass == EMPTY_PASS)
            return true;
        pass_prop pass_property = num_to_prop_.at(pass);

        return can_be_applied(pass, pass_property);
    }

    int apply_pass(int pass)
    {
        if (pass == EMPTY_PASS)
            return 0;
        passes_.push_back(pass);
        pass_prop pass_property = num_to_prop_.at(pass);

        if (!can_be_applied(pass, pass_property))
        {
            return -1;
        }

        custom_property_state |= pass_property.custom.provided;
        custom_property_state &= ~pass_property.custom.destroyed;

        original_property_state |= pass_property.original.provided;
        original_property_state &= ~pass_property.original.destroyed;

        return 0;
    }

    int revert_applying(int pass)
    {
        pass_prop pass_prop = num_to_prop_.at(pass);

        custom_property_state |= pass_prop.custom.destroyed;
        custom_property_state &= ~pass_prop.custom.provided;

        original_property_state |= pass_prop.original.destroyed;
        original_property_state &= ~pass_prop.original.provided;

        return 0;
    }

private:
    bool can_be_applied(int pass, const pass_prop property)
    {
        if (((original_property_state & property.original.required) != property.original.required) ||
            ((custom_property_state & property.custom.required) != property.custom.required))
        {
            // std::cout << "rejected " << pass << std::endl;
            return false;
        }
        return true;
    }

};


class PassListGenerator
{
    std::vector<pass_info> info_vec_;
    std::unordered_map<std::string, int> name_to_id_map_; // we give each pass an id and work with ids to avoid needless heap indirection
    std::unordered_map<int, std::string> id_to_name;

    std::unordered_map<int, pass_prop> pass_to_properties_; // hash map: pass id -> it's properties

    std::unordered_map<std::pair<unsigned long, unsigned long>, std::vector<int>> unique_requirement_to_passes_;

    PropertyStateMachine state;

    std::vector<int> generated_sequence;
    std::vector<int> given_sequence;

    static constexpr int TRY_AMOUNT = 1e4;
    static constexpr int MAX_PASS_AMOUNT = 400;

public:
    bool include_used = false;

public:

    PassListGenerator() = default;

    template <typename iter_info>
    PassListGenerator(iter_info begin_info, iter_info end_info) :
    info_vec_{begin_info, end_info}
    {
        setup_structures();
    }

    template <typename iter>
    void set_info_vec(iter begin, iter end)
    {
        info_vec_ = {begin, end};
    }

    template <typename iter>
    void set_start_seq(iter begin, iter end)
    {
        given_sequence = {begin, end};
    }

    template <typename iter_in, typename iter_out>
    void map_names_onto_id(iter_in input_begin, iter_in input_end, iter_out output_begin)
    {
        auto&& mapper = [this](const std::string& str){ return name_to_id_map_[str]; };
        std::transform(input_begin, input_end, output_begin, mapper);
    }

    int map_name_onto_id(const std::string& str)
    {
        return name_to_id_map_[str];
    }

    template <typename iter_in, typename iter_out>
    void map_id_onto_names(iter_in input_begin, iter_in input_end, iter_out output_begin)
    {
        auto&& mapper = [this](int id){ return id_to_name[id]; };
        std::transform(input_begin, input_end, output_begin, mapper);
    }

    // map passes' names onto ids and batches of passes onto ids
    void setup_structures();

    template <typename iter>
    void get_new_action_space(iter begin, iter end, const std::pair<unsigned long, unsigned long>& start_prop)
    {
        generated_sequence.clear();
        get_prop_state(begin, end, start_prop);

        const auto original_state = state.original_property_state;
        const auto custom_state = state.custom_property_state;

        for (auto&& pass : given_sequence)
        {
            auto&& both_props = pass_to_properties_[pass];

            auto&& original_required = both_props.original.required;
            auto&& custom_required = both_props.custom.required;

            // std::cout << "Checking " << std::string(*begin) << " with " << original_required << ' ' << custom_required << std::endl;

            if (((original_required & original_state) == original_required) && ((custom_required & custom_state) == custom_required)
                && (include_used || (std::find(begin, end, pass) == end)))
            {
                generated_sequence.push_back(pass);
            }
            // else
            // {
            //     std::cout << std::string(*begin) << " no good: " << (original_required & original_state) << ' ' << (custom_required & custom_state) << std::endl;
            // }

        }
    }

    template <typename iter>
    std::pair<unsigned long, unsigned long> get_prop_state(iter begin, iter end, const std::pair<unsigned long, unsigned long>& start_prop)
    {
        state.passes_.clear();
        state.original_property_state = start_prop.first;
        state.custom_property_state = start_prop.second;

        for (; begin != end; begin++)
        {
            state.apply_pass(*begin);
        }
        return {state.original_property_state, state.custom_property_state};
    }

    template <typename iter>
    int valid_pass_seq(iter begin, iter end, const std::pair<unsigned long, unsigned long>& start_prop, unsigned long ending_state)
    {
        state.passes_.clear();
        state.original_property_state = start_prop.first;
        state.custom_property_state = start_prop.second;

        auto&& finder = [this](int pass_id)
        {
            if (pass_id == 0)
                return false;

            return state.apply_pass(pass_id) != 0;
        };

        auto&& bad_pass = std::find_if(begin, end, finder);

        if (bad_pass != end)
        {
            return bad_pass - begin + 1;
        }

        if ((state.custom_property_state & ending_state) != ending_state)
            return end - begin;

        return 0;
    }

    template <typename iter>
    int check_loop2(iter begin, iter end)
    {
        if (std::find(begin, end, name_to_id_map_["loop2"]) == end)
            return end - begin + 1;

        return 0;
    }

    template<typename iter>
    void make_valid_pass_seq(iter begin, iter end, const std::pair<unsigned long, unsigned long>& start_prop, unsigned long ending_state)
    {
        generated_sequence.clear();
        state.original_property_state = start_prop.first;
        state.custom_property_state = start_prop.second;

        for (; begin != end; begin++)
        {
            generated_sequence.push_back(*begin);
            state.apply_pass(*begin);
        }

        std::stack<int> passes;

        auto ending_state_diff = ending_state & (~state.custom_property_state);
        auto&& necessary_pass_finder = [&ending_state_diff](const pass_info& info){return (info.prop.custom.provided & ending_state_diff) == ending_state_diff && 
                                                                                            (info.prop.custom.destroyed & ending_state_diff) != ending_state_diff;};

        while (ending_state_diff != 0)
        {
            auto&& pass_info_it = std::find_if(info_vec_.begin(), info_vec_.end(), necessary_pass_finder);
            passes.push(name_to_id_map_[pass_info_it->name]);

            ending_state_diff = pass_info_it->prop.custom.required & (~state.custom_property_state);
        }

        for (int i = 0; !passes.empty(); i++)
        {
            generated_sequence.push_back(passes.top());
            passes.pop();
        }
    }

    std::unordered_set<std::pair<unsigned long, unsigned long>> get_unique_requirements();

    void generate_prop_passes_map();

    void get_action_space_by_property(const std::pair<unsigned long, unsigned long>& prop_state);

    int shuffle_pass_order(const std::pair<unsigned long, unsigned long>& initial_property_state,
                           const std::pair<unsigned long, unsigned long>& ending_property_state);


    using iterator = std::vector<int>::iterator;
    using const_iterator = std::vector<int>::const_iterator;

    iterator begin() { return generated_sequence.begin(); }
    iterator end() { return generated_sequence.end(); }
    const_iterator begin() const { return generated_sequence.begin(); }
    const_iterator end() const { return generated_sequence.end(); }
    const_iterator cbegin() { return generated_sequence.cbegin(); }
    const_iterator cend() { return generated_sequence.cend(); }
};

} // namespace gcc_reorder

#endif