#pragma once

#include <stdexcept>

#include <vector>
#include <string>

#include <tuple>
#include <algorithm>
#include <random>
#include <optional>

#include <intdef>

enum class rotation : u32 {
    cw = 1, _180, ccw
};

enum class mino_type : u32 {
    I, J, L, O, S, T, Z
};

struct collision_t {
    u32 left = 0, right = 0, down = 0, up = 0;

    collision_t() = default;

    collision_t(u32 l, u32 r, u32 d, u32 u)
    : left(l), right(r), down(d), up(u) { }
};

struct tetromino {
private:
    tetromino() = default;

    tetromino(
        mino_type __type,
        const std::vector<std::vector<i8>>& __mino
    )
    : _M_mino(__mino), _M_type(__type), _M_size(__mino.size())
    { _M_calculate_collision(); }

public:
    tetromino(const tetromino&) = default;
    tetromino(tetromino&&) = default;

private:
    using container_type = std::vector<std::vector<i8>>;

    container_type _M_mino;
    mino_type _M_type;
    u32 _M_size, _M_direction = 0;

    collision_t _M_collision;

private:
    void _M_rotate_cw() {
        container_type __tmp = _M_mino;

        for (u32 __i = 0; __i < _M_size; __i++) {
            for (u32 __j = 0; __j < _M_size; __j++) {
                _M_mino[__j][_M_size - __i - 1] = __tmp[__i][__j];
            }
        }
    }

    void _M_rotate_ccw() {
        container_type __tmp = _M_mino;

        for (u32 __i = 0; __i < _M_size; __i++) {
            for (u32 __j = 0; __j < _M_size; __j++) {
                _M_mino[_M_size - __j - 1][__i] = __tmp[__i][__j];
            }
        }
    }

    void _M_rotate_180() {
        container_type __tmp = _M_mino;

        for (u32 __i = 0; __i < _M_size; __i++) {
            for (u32 __j = 0; __j < _M_size; __j++) {
                _M_mino[_M_size - __i - 1][_M_size - __j - 1] = __tmp[__i][__j];
            }
        }
    }

    void _M_calculate_collision() {
        _M_collision.left = _M_collision.up = _M_size;
        _M_collision.right = _M_collision.down = 0;

        for (u32 __i = 0; __i < _M_size; __i++) {
            for (u32 __j = 0; __j < _M_size; __j++) {
                if (_M_mino[__i][__j] != 0) {
                    _M_collision.left  = std::min(_M_collision.left,  __j);
                    _M_collision.right = std::max(_M_collision.right, __j);
                    _M_collision.down  = std::max(_M_collision.down,  __i);
                    _M_collision.up    = std::min(_M_collision.up,    __i);
                }
            }
        }
    }

public:
    void rotate(rotation __r) {
        switch (__r) {
            case rotation::cw  : _M_rotate_cw (); break;
            case rotation::ccw : _M_rotate_ccw(); break;
            case rotation::_180: _M_rotate_180(); break;
            default: return;
        }

        _M_direction = (_M_direction + static_cast<u32>(__r)) % 4;

        _M_calculate_collision();
    }

    void set_direction(u32 __d) {
        u32 __k = (4 + __d - _M_direction) % 4;

        rotate(static_cast<rotation>(__k));
    }

    constexpr mino_type type() const { return _M_type; }
    constexpr u32 direction() const { return _M_direction; }
    constexpr u32 size() const { return _M_size; }
    const std::vector<std::vector<i8>>& data() const { return _M_mino; }

    constexpr collision_t collision() const
    { return _M_collision; }

    constexpr char to_char() const { return static_cast<char>(*this); }

    tetromino& operator=(const tetromino&) = default;
    tetromino& operator=(tetromino&&) = default;

    bool operator==(const tetromino& __t) const
    { return _M_type == __t._M_type; }
    bool operator!=(const tetromino& __t) const
    { return !(*this == __t); }

    constexpr explicit operator char() const {
        switch (_M_type) {
            case mino_type::I: return 'I';
            case mino_type::J: return 'J';
            case mino_type::L: return 'L';
            case mino_type::O: return 'O';
            case mino_type::S: return 'S';
            case mino_type::T: return 'T';
            case mino_type::Z: return 'Z';
            default: return '?';
        }
    }

public:
    static const tetromino I;
    static const tetromino J;
    static const tetromino L;
    static const tetromino O;
    static const tetromino S;
    static const tetromino T;
    static const tetromino Z;

    static std::optional<tetromino> from_char(char __c) {
        switch (std::toupper(__c)) {
            case 'I': return I;
            case 'J': return J;
            case 'L': return L;
            case 'O': return O;
            case 'S': return S;
            case 'T': return T;
            case 'Z': return Z;
            default: return std::nullopt;
        }
    }

    /**
     * @brief String to Tetromino Conversion Rules
     *
     * - Commas (,) and whitespace are ignored.
     *
     * 1. A single character { I, J, L, O, S, T, Z } is converted to the corresponding tetromino.
     *    If an invalid character is provided, it will return std::nullopt.
     * 
     * 2. Brackets like '[IO]' mean: choose 1 random tetromino from the set inside the brackets.
     *    If brackets are not closed, it will return std::nullopt,
     *    and also an empty set will return std::nullopt.
     * 
     * 3. If there is a '^' at the start inside brackets, like '[^IO]',
     *    choose a random tetromino from the set except those tetrominoes.
     * 
     * 4. If there is 'p' and a number `N` after the brackets, like '[IO]p2',
     *    choose `N` random tetrominoes from the set (no duplicates).
     *    If `N` is 0 or greater than the size of the set, it will return std::nullopt,
     *    and also if there's no bracket before 'p', it will return std::nullopt.
     * 
     * 5. If there is '!' after the brackets, like '[TIOS]!',
     *    choose all tetrominoes from the set and shuffle the order.
     *    (This is the same as '[TIOS]p4' if the set has 4 elements.)
     * 
     * 6. '*' is the same as [IJLOSTZ].
     *
     * @example
     *   "IJL"         → I, J, L
     *   "[SZO]p2"     → 2 random tetrominoes from S, Z and O
     *   "[^O]!"       → All tetrominoes except O, shuffled
     *   "*"           → 1 random tetromino (I, J, L, O, S, T, Z)
     */
    static std::optional<std::vector<tetromino>> gen(const std::string& __s);
};