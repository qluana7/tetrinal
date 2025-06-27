#pragma once

#include <concepts>

#include <array>

#include <algorithm>

#include <rules/tetromino.hpp>
#include <rules/attack_table.hpp>
#include <rules/field.hpp>

struct spin_info {
    tetromino _M_mino;
    i32 _M_x, _M_y;
    u32 _M_table_idx;
    bool _M_immobile;

    const field& _M_field;
};

template <typename T>
concept spin_rule = requires (spin_info __info) {
    { T::get(__info) } -> std::same_as<spin_type>;
};

namespace spin_tables {

struct tspin {
    static constexpr spin_type get(spin_info __info) {
        if (__info._M_mino.type() != mino_type::T) return spin_type::NONE;

        // Use 3 corner rule
        
        // Base on direction of T mino.
        // left_top, right_top, right_bottom, left_bottom
        std::array<bool, 4> __corner = {
            __info._M_field.get_block(__info._M_x, __info._M_y) != block_type::EMPTY,
            __info._M_field.get_block(__info._M_x + 2, __info._M_y) != block_type::EMPTY,
            __info._M_field.get_block(__info._M_x + 2, __info._M_y - 2) != block_type::EMPTY,
            __info._M_field.get_block(__info._M_x, __info._M_y - 2) != block_type::EMPTY
        };

        switch (__info._M_mino.direction()) {
            case 0: break;
            case 1: std::rotate(__corner.begin(), __corner.begin() + 1, __corner.end()); break;
            case 2: std::rotate(__corner.begin(), __corner.begin() + 2, __corner.end()); break;
            case 3: std::rotate(__corner.begin(), __corner.begin() + 3, __corner.end()); break;
        }

        u32 __cnt = std::count(__corner.begin(), __corner.end(), true);

        if (__cnt < 3) return spin_type::NONE;

        // if left_top + right_top -> SPIN
        // else -> MINI
        if (__corner[0] && __corner[1]) return spin_type::SPIN;
        else return spin_type::MINI;
    }
};

// t-spin + (immobile t -> t-mini)
struct tspin_plus {
    static constexpr spin_type get(spin_info __info) {
        if (__info._M_mino.type() != mino_type::T) return spin_type::NONE;

        spin_type __sp = tspin::get(__info);

        if (__sp == spin_type::NONE && __info._M_immobile)
            return spin_type::MINI;
        else return __sp;
    }
};

struct all_spin {
    static constexpr spin_type get(spin_info __info) {
        if (__info._M_mino.type() == mino_type::T)
            return tspin::get(__info);
        else
            return __info._M_immobile ? spin_type::SPIN : spin_type::NONE;
    }
};

struct all_spin_plus {
    static constexpr spin_type get(spin_info __info) {
        if (__info._M_mino.type() == mino_type::T)
            return tspin_plus::get(__info);
        else
            return __info._M_immobile ? spin_type::SPIN : spin_type::NONE;
    }
};

struct all_mini {
    static constexpr spin_type get(spin_info __info) {
        if (__info._M_mino.type() == mino_type::T)
            return tspin::get(__info);
        else
            return __info._M_immobile ? spin_type::MINI : spin_type::NONE;
    }
};

struct all_mini_plus {
    static constexpr spin_type get(spin_info __info) {
        if (__info._M_mino.type() == mino_type::T)
            return tspin_plus::get(__info);
        else
            return __info._M_immobile ? spin_type::MINI : spin_type::NONE;
    }
};

// TODO : Add tspin_plus, all_spin, all_mini, etc...

}