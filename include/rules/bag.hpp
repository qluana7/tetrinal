#pragma once

#include <concepts>

#include <vector>

#include <memory>
#include <algorithm>
#include <random>

#include <intdef>

#include <rules/tetromino.hpp>

/* interface */ struct Ibag {
    virtual std::vector<tetromino> generate(std::mt19937& __rand) = 0;
};

namespace bags {

enum class types {
    bag7, bag14, bag7x, bag_classic
};

struct bag7 : Ibag {
    std::vector<tetromino> generate(std::mt19937& __rand) override {
        std::vector<tetromino> __bag = {
            tetromino::I, tetromino::J, tetromino::L,
            tetromino::O, tetromino::S, tetromino::T,
            tetromino::Z
        };

        std::shuffle(__bag.begin(), __bag.end(), __rand);
        return __bag;
    }
};

struct bag14 : Ibag {
    std::vector<tetromino> generate(std::mt19937& __rand) override {
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

struct bag7x : Ibag {
public:
    u32 _M_n = 1;

    std::vector<tetromino> generate(std::mt19937& __rand) override {
        std::vector<tetromino> __bag = {
            tetromino::I, tetromino::J, tetromino::L,
            tetromino::O, tetromino::S, tetromino::T,
            tetromino::Z
        };

        std::vector<tetromino> __extra;

        std::uniform_int_distribution<std::size_t> __idx(0, 6);

        for (u32 __i = 0; __i < _M_n; __i++)
            __extra.push_back(__bag[__idx(__rand)]);

        __bag.insert(__bag.end(), __extra.begin(), __extra.end());
        std::shuffle(__bag.begin(), __bag.end(), __rand);
        return __bag;
    }
};

struct bag_classic : Ibag {
    std::vector<tetromino> generate(std::mt19937& __rand) override {
        std::vector<tetromino> __bag = {
            tetromino::I, tetromino::J, tetromino::L,
            tetromino::O, tetromino::S, tetromino::T,
            tetromino::Z
        };

        return { __bag[std::uniform_int_distribution<std::size_t>(0, 6)(__rand)] };
    }
};

std::unique_ptr<Ibag> create(types __type) {
    switch (__type) {
        case types::bag7:        return std::make_unique<bag7>();
        case types::bag14:       return std::make_unique<bag14>();
        case types::bag7x:       return std::make_unique<bag7x>();
        case types::bag_classic: return std::make_unique<bag_classic>();
        default:                 return nullptr;
    }
}

}

struct bag_generator {
    bag_generator(std::mt19937& __rand, std::unique_ptr<Ibag> __bag)
    : _M_rand(__rand), _M_current(_M_queue.end()), _M_bag(std::move(__bag)) { }

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
    bag_generator(const _Container& __c, std::mt19937& __rand, std::unique_ptr<Ibag> __bag)
    : _M_rand(__rand), _M_queue(__c.begin(), __c.end()), _M_current(_M_queue.begin())
    , _M_bag(std::move(__bag)) { }

private:
    using container_type = std::vector<tetromino>;

    std::mt19937 _M_rand;
    container_type _M_queue;
    container_type::const_iterator _M_current;
    std::unique_ptr<Ibag> _M_bag;

public:
    tetromino next() {
        if (_M_current == _M_queue.end()) {
            _M_queue = _M_bag->generate(_M_rand);
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