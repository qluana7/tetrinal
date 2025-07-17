#pragma once

#include <concepts>

#include <array>
#include <vector>

#include <memory>
#include <numeric>

#include <lib/intdef>

#include <rules/tetromino.hpp>

namespace kick_tables::detail {
    using pos_t = std::pair<i32, i32>;
    using container_t = std::vector<std::vector<pos_t>>;
    using kick_table_t = std::vector<pos_t>;
}

/* interface */ struct Ikick_table {
    virtual const kick_tables::detail::kick_table_t& get(
        tetromino __t, u32 __from, u32 __to
    ) const = 0;
};

namespace kick_tables::detail {
    /* Table rule
    
    directions : 0, R, 2, L

    table order :
        0 -> R, R -> 0, R -> 2, 2 -> R,
        2 -> L, L -> 2, L -> 0, 0 -> L,
        0 -> 2, 2 -> 0, R -> L, L -> R

    */
    template <typename T>
    concept general_kick_table_data_t = requires () {
        { T::_M_table } -> std::same_as<const container_t&>;
        { T::_M_table_I } -> std::same_as<const container_t&>;
    };

    const kick_table_t empty = {};

    template <general_kick_table_data_t T>
    const kick_table_t& general_get(
        tetromino __t, u32 __from, u32 __to
    ) {
        if (__from > 3 || __to > 3) return empty;

        if (__from == __to) return empty;

        if (std::abs((i32)__from - (i32)__to) == 2) {
            u32 __idx = __from / 2 + (__from & 1) * 2 + 8;

            if (__t.type() == mino_type::I)
                return T::_M_table_I[__idx];
            else
                return T::_M_table[__idx];
        }

        bool __is_cw = (__from + 1) % 4 == __to;
        if (!__is_cw) std::swap(__from, __to);

        u32 __idx = __from * 2 + !__is_cw;

        if (__t.type() == mino_type::I)
            return T::_M_table_I[__idx];
        else
            return T::_M_table[__idx];
    }

}

namespace kick_tables {

enum class types {
    srs,        // Default SRS
    srs_plus,   // Tetrio SRS+
    srs_x,      // Tetrio SRS-X
};

// Default SRS, but 180 spin is custom version of Tetrio
struct srs : Ikick_table {
    static const detail::container_t _M_table;

    static const detail::container_t _M_table_I;

    const detail::kick_table_t& get(
        tetromino __t, u32 __from, u32 __to
    ) const override { return detail::general_get<srs>(__t, __from, __to); }
};

// from tetrio SRS+
struct srs_plus : public srs {
    static const detail::container_t _M_table_I;

    const detail::kick_table_t& get(
        tetromino __t, u32 __from, u32 __to
    ) const override { return detail::general_get<srs_plus>(__t, __from, __to); }
};

// from tetrio SRS-X
struct srs_x : Ikick_table {
    static const detail::container_t _M_table;

    static const detail::container_t _M_table_I;

    const detail::kick_table_t& get(
        tetromino __t, u32 __from, u32 __to
    ) const override { return detail::general_get<srs_x>(__t, __from, __to); }
};

// TODO List : nullpomino_180, classic, asc
// nullpomino : https://github.com/nullpomino/nullpomino/blob/master/nullpomino-core/src/main/java/mu/nu/nullpo/game/subsystem/wallkick/StandardMild180Wallkick.java

// wishlist : jstris (no information available yet)

std::unique_ptr<Ikick_table> create(types __type);

}