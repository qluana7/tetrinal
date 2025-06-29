#pragma once

#include <vector>
#include <list>
#include <tuple>

#include <memory>
#include <random>
#include <algorithm>

#include <cmath>

#include <intdef>

#include <rules/tetromino.hpp>

enum class block_type : u8
{ I, J, L, O, S, T, Z, GARBAGE, WALL, EMPTY };

enum class block_attribute : u8
{ NORMAL, GUIDE, LOCKED };

/* interface */ struct Iblock_color {
    virtual constexpr std::tuple<u8, u8, u8> color(block_type __bt) = 0;
};

namespace block_color {

enum class types {
    classic, // Classic Tetris colors
    bright,  // Bright colors
};

struct classic : Iblock_color {
    constexpr std::tuple<u8, u8, u8> color(block_type __bt) override {
        switch (__bt) {
            case block_type::I: return { 0, 255, 255 };
            case block_type::J: return { 0, 0, 255 };
            case block_type::L: return { 255, 165, 0 };
            case block_type::O: return { 255, 255, 0 };
            case block_type::S: return { 0, 255, 0 };
            case block_type::T: return { 255, 0, 255 };
            case block_type::Z: return { 255, 0, 0 };
            case block_type::GARBAGE: return { 128, 128, 128 };
            case block_type::WALL: return { 192, 192, 192 };
            default: return { 0, 0, 0 };
        }
    }
};

struct bright : Iblock_color {
    constexpr std::tuple<u8, u8, u8> color(block_type __bt) override {
        switch (__bt) {
            case block_type::I: return { 68, 255, 255 };
            case block_type::J: return { 68, 68, 255 };
            case block_type::L: return { 255, 204, 68 };
            case block_type::O: return { 255, 255, 68 };
            case block_type::S: return { 68, 255, 68 };
            case block_type::T: return { 255, 68, 255 };
            case block_type::Z: return { 255, 68, 68 };
            case block_type::GARBAGE: return { 128, 128, 128 };
            case block_type::WALL: return { 192, 192, 192 };
            default: return { 0, 0, 0 };
        }
    }
};

std::unique_ptr<Iblock_color> create(types __type) {
    switch (__type) {
        case types::classic: return std::make_unique<classic>();
        case types::bright:  return std::make_unique<bright>();
        default: return nullptr;
    }
}

}

struct field {
private:
    using cell_type = std::pair<block_type, block_attribute>;
    using field_type = std::vector<std::vector<cell_type>>;

    u32 _M_width = 10;
    u32 _M_height = 24;

    field_type _M_field;

public:
    field(u32 __width = 10, u32 __height = 24)
    : _M_width(__width), _M_height(__height),
      _M_field(__height, std::vector<cell_type>(__width, { block_type::EMPTY, block_attribute::NORMAL })) { }

private:
    void _M_calcuate_attribute() {
        for (auto& __f : _M_field) {
            if (std::all_of(__f.begin(), __f.end(), [] (cell_type __bt) {
                auto [__blk, __attr] = __bt;

                return static_cast<u8>(__blk) < 7 && 
                       (__attr == block_attribute::NORMAL ||
                       __attr == block_attribute::LOCKED);
            })) {
                for (auto& [_, __attr] : __f)
                    __attr = block_attribute::LOCKED;
            } else {
                for (auto& [_, __attr] : __f) {
                    if (__attr == block_attribute::LOCKED)
                        __attr = block_attribute::NORMAL;
                }
            }
        }
    }

public:
    void clear() {
        for (auto& row : _M_field) {
            std::fill(row.begin(), row.end(), cell_type { block_type::EMPTY, block_attribute::NORMAL });
        }
    }

    void set_block(
        u32 __x, u32 __y, tetromino __t,
        block_attribute __attr = block_attribute::NORMAL
    ) {
        if (__x < _M_width && __y < _M_height) {
            _M_field[__y][__x] = { _S_tetromino_to_block_type(__t), __attr };
        }
    }

    block_type get_block(i32 __x, i32 __y) const {
        if (
            __x >= 0 && __y >= 0 &&
            (u32)__x < _M_width && (u32)__y < _M_height
        ) {
            auto __attr = _M_field[__y][__x].second;

            if (__attr == block_attribute::GUIDE)
                return block_type::EMPTY;

            return _M_field[__y][__x].first;
        }
        return block_type::WALL; // Out of bounds
    }

    void remove_row(u32 __y) {
        if (__y < _M_height) {
            _M_field.erase(_M_field.begin() + __y);
            _M_field.emplace_back(_M_width, cell_type { block_type::EMPTY, block_attribute::NORMAL });
        }
    }

    u32 proceed_lines() {
        u32 cnt = 0;

        for (u32 __y = 0; __y < _M_height; ++__y) {
            if (std::all_of(_M_field[__y].begin(), _M_field[__y].end(), [] (cell_type __bt) {
                return __bt.first != block_type::EMPTY;
            })) {
                remove_row(__y); __y--; cnt++;
            }
        }

        return cnt;
    }

    void put_garbage(u32 __cnt, i32 __hole = -1) {
        if (__cnt == 0 || __cnt > _M_height) return;

        if ((u32)__hole >= _M_width) {
            std::mt19937 __mt{std::random_device{}()}; 
            __hole = std::uniform_int_distribution<u32>(0, _M_width - 1)(__mt);
        }
            
        field_type nr;
        for (u32 i = 0; i < __cnt; ++i) {
            std::vector<cell_type> row(_M_width, { block_type::GARBAGE, block_attribute::NORMAL });
            row[__hole].first = block_type::EMPTY;
            
            nr.push_back(row);
        }
        
        _M_field.insert(_M_field.begin(), nr.begin(), nr.end());

        if (_M_field.size() > _M_height)
            _M_field.resize(_M_height);
    }

    // start point is left, top of tetromino.
    void put_mino(i32 __x, i32 __y, const tetromino& __t, bool __guide = false) {
        for (i32 i = 0; i < (i32)__t.size(); ++i) {
            if (__x + i < 0 || __x + i >= (i32)_M_width) continue;

            for (i32 j = 0; j < (i32)__t.size(); ++j) {
                if (__y - j < 0 || __y - j >= (i32)_M_height) continue;

                if (__t.data()[j][i] != 0) {
                    _M_field[__y - j][__x + i].first = _S_tetromino_to_block_type(__t);

                    if (__guide) {
                        _M_field[__y - j][__x + i].second = block_attribute::GUIDE;
                    } else {
                        _M_field[__y - j][__x + i].second = block_attribute::NORMAL;
                    }
                }
            }
        }
    }

    bool is_empty() const {
        return std::all_of(_M_field.begin(), _M_field.end(), [] (const std::vector<cell_type>& row) {
            return std::all_of(row.begin(), row.end(), [] (const cell_type& cell) {
                return cell.first == block_type::EMPTY;
            });
        });
    }

    u32 width() const { return _M_width; }
    u32 height() const { return _M_height; }

    const field_type& data() const { return _M_field; }

private:
    static block_type _S_tetromino_to_block_type(tetromino __t) {
        switch (__t.type()) {
            case mino_type::I: return block_type::I;
            case mino_type::J: return block_type::J;
            case mino_type::L: return block_type::L;
            case mino_type::O: return block_type::O;
            case mino_type::S: return block_type::S;
            case mino_type::T: return block_type::T;
            case mino_type::Z: return block_type::Z;
            default: return block_type::EMPTY;
        }
    }
};