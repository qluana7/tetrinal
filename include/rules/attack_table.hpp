#pragma once

#include <concepts>

#include <array>
#include <string>

#include <memory>

#include <cmath>

#include <lib/intdef>

enum class attack_type : u32 {
    SINGLE, DOUBLE, TRIPLE, QUAD
};

enum class spin_type : u32 {
    NONE, MINI, SPIN
};

struct attack_info {
    attack_type _M_type;
    i32 _M_combo, _M_btb;
    spin_type _M_spin;

    bool _M_pc;

    std::string to_string(char mino) const {
        std::string __str;
        
        if (_M_spin != spin_type::NONE) {
            if (_M_spin == spin_type::MINI)
                __str += "MINI ";
            __str += mino + (std::string)"-SPIN ";
        }

        switch (_M_type) {
            case attack_type::SINGLE: __str += "SINGLE"; break;
            case attack_type::DOUBLE: __str += "DOUBLE"; break;
            case attack_type::TRIPLE: __str += "TRIPLE"; break;
            case attack_type::QUAD: __str += "QUAD"; break;
        }

        if (_M_pc)
            __str += "!!";

        return __str;
    }
};

/* interface */ struct Iattack_table
{ virtual constexpr u32 get(attack_info __atk) = 0; };

namespace attack_tables {

enum class types {
    tetrio
};

struct tetrio : Iattack_table {
private:
    static constexpr u32 B2B_BONUS = 1;
    static constexpr f64 B2B_BONUS_LOG = .8;
    static constexpr u32 MINIFIER = 1;
    static constexpr f64 MINIFIER_LOG = 1.25;
    static constexpr f64 COMBO_BONUS = .25;

    static constexpr std::array<std::array<u32, 4>, 3> __table = {{
        // NONE
        { 0, 1, 2, 4 },
        // MINI
        { 0, 1, 3, 4 },
        // SPIN
        { 2, 4, 6, 10 }
    }};

public:
    constexpr u32 get(attack_info __atk) override {
        auto [__t, __c, __b, __s, _] = __atk;

        f64 __r = __table[static_cast<u32>(__s)][static_cast<u32>(__t)];

        if (__b > 0) {
            f64 __tmp = 1 + std::log1p(__b * B2B_BONUS_LOG);

            __r += (
                B2B_BONUS * (
                    (u32)(__tmp) +
                    (__b == 1 ? 0 : (__tmp - (u32)(__tmp) + 1) / 3)
                )
            );
        }

        __r *= 1 + COMBO_BONUS * __c;

        if (__c > 1)
            __r = std::max(__r, std::log1p(MINIFIER * __c * MINIFIER_LOG));
        
        return (u32)std::floor(__r) + (__atk._M_pc * 10);
    }
};

std::unique_ptr<Iattack_table> create(types __type) {
    switch (__type) {
        case types::tetrio: return std::make_unique<tetrio>();
        default: return nullptr;
    }
}

}
