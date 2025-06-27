#pragma once

#include <concepts>

#include <vector>

#include <algorithm>
#include <random>

#include <intdef>

#include <rules/tetromino.hpp>

template <typename T, typename _RandomFunc>
concept bag_rule = requires (_RandomFunc& __rand) {
    { T::generate(__rand) } -> std::same_as<std::vector<tetromino>>;
};

struct bag7 {
    template <typename _RandomFunc>
    static std::vector<tetromino> generate(_RandomFunc& __rand) {
        std::vector<tetromino> __bag = {
            tetromino::I, tetromino::J, tetromino::L,
            tetromino::O, tetromino::S, tetromino::T,
            tetromino::Z
        };

        std::shuffle(__bag.begin(), __bag.end(), __rand);
        return __bag;
    }
};

struct bag14 {
    template <typename _RandomFunc>
    static std::vector<tetromino> generate(_RandomFunc& __rand) {
        std::vector<tetromino> __bag = {
            tetromino::I, tetromino::J, tetromino::L,
            tetromino::O, tetromino::S, tetromino::T,
            tetromino::Z, tetromino::I, tetromino::J,
            tetromino::L, tetromino::O, tetromino::S,
            tetromino::T, tetromino::Z
        };

        std::shuffle(__bag.begin(), __bag.end(), __rand);
        return __bag;
    }
};

template <u32 _N = 1>
struct bag7x {
public:
    template <typename _RandomFunc>
    static std::vector<tetromino> generate(_RandomFunc& __rand) {
        std::vector<tetromino> __bag = {
            tetromino::I, tetromino::J, tetromino::L,
            tetromino::O, tetromino::S, tetromino::T,
            tetromino::Z
        };

        std::vector<tetromino> __extra;

        std::uniform_int_distribution<std::size_t> __idx(0, 6);

        for (u32 __i = 0; __i < _N; __i++)
            __extra.push_back(__bag[__idx(__rand)]);

        __bag.insert(__bag.end(), __extra.begin(), __extra.end());
        std::shuffle(__bag.begin(), __bag.end(), __rand);
        return __bag;
    }
};

struct bag_classic {
    template <typename _RandomFunc>
    static std::vector<tetromino> generate(_RandomFunc& __rand) {
        std::vector<tetromino> __bag = {
            tetromino::I, tetromino::J, tetromino::L,
            tetromino::O, tetromino::S, tetromino::T,
            tetromino::Z
        };

        return { __bag[std::uniform_int_distribution<std::size_t>(0, 6)(__rand)] };
    }
};

template <typename _Rule = bag7, typename _RandomFunc = std::mt19937>
requires bag_rule<_Rule, _RandomFunc>
struct bag {
    bag(_RandomFunc __rand = _RandomFunc{std::random_device{}()})
    : _M_rand(__rand), _M_current(_M_queue.end()) { }

    /**
     * @brief Constructs a bag and insert a predefined set of tetrominoes before bag start.
     * 
     * @param __c A container of tetrominoes to be inserted into the bag.
     * @param __rand A random number generator to shuffle the bag.
     * @tparam _Container A container type that supports range operations.
     * 
     * This constructor allows you to initialize the bag with a specific set of tetrominoes,
     * which can be useful for testing or specific game modes. The provided container is copied
     * into the bag, and the bag is shuffled using the provided random number generator.
     * 
     * @note The container must support range operations, such as `begin()` and `end()`.
     * 
     */
    template <typename _Container>
    requires std::ranges::range<_Container>
    bag(const _Container& __c, _RandomFunc __rand = _RandomFunc{std::random_device{}()})
    : _M_rand(__rand), _M_queue(__c.begin(), __c.end()), _M_current(_M_queue.begin()) { }

private:
    using container_type = std::vector<tetromino>;

    _RandomFunc _M_rand;
    container_type _M_queue;
    container_type::const_iterator _M_current;

public:
    tetromino next() {
        if (_M_current == _M_queue.end()) {
            _M_queue = _Rule::generate(_M_rand);
            _M_current = _M_queue.begin();
        }
        
        tetromino __next = *_M_current;
        _M_current++;
        return __next;
    }

    void reset() {
        _M_queue.clear();
        _M_current = _M_queue.end();
    }
};