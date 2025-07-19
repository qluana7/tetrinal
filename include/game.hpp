#pragma once

#include <array>
#include <list>
#include <map>
#include <deque>
#include <sstream>

#include <memory>
#include <functional>
#include <atomic>
#include <random>
#include <chrono>
#include <thread>
#include <optional>

#include <cstddef>

#include <ncurses.h>

#include <lib/intdef>

#include <config.hpp>
#include <rules/tetromino.hpp>
#include <rules/attack_table.hpp>
#include <rules/kick_table.hpp>
#include <rules/spin.hpp>
#include <rules/bag.hpp>
#include <rules/field.hpp>

#include <util/conv.hpp>
#include <util/buffer.hpp>

class game {
public:
    using clock_type = std::chrono::high_resolution_clock;
    using time_type = clock_type::time_point;

    using control_key = user_config::control_config::KEYS;

    using puzzle_function = std::function<bool (
        // Field
        const field&,
        // Last placed tetromino
        tetromino,
        // Last placed tetromino position
        i32, i32,
        // Current attack info
        attack_info
    )>;

public:
    game(
        std::mt19937& __rand,
        user_config __uconf = user_config{},
        block_color::types __color = block_color::types::bright,
        bags::types __bag_type = bags::types::bag7
    ) : _M_user_config(__uconf), _M_rand(__rand),
        _M_bag(__rand, bags::create(__bag_type)),
        _M_color(block_color::create(__color)) {
        _M_field = field(__uconf.field.width, __uconf.field.height + __uconf.field.extra_height);

        _M_attack_table = attack_tables::create(__uconf.game.attack_table);
        _M_kick_table = kick_tables::create(__uconf.game.kick_table);
        _M_spin_table = spin_tables::create(__uconf.game.spin_table);

        _M_init();
        reset();
    }

    ~game() {
        if (_M_windows._M_field) delwin(_M_windows._M_field);
        if (_M_windows._M_next) delwin(_M_windows._M_next);
        if (_M_windows._M_hold) delwin(_M_windows._M_hold);
        if (_M_windows._M_stats) delwin(_M_windows._M_stats);
        if (_M_windows._M_msg) delwin(_M_windows._M_msg);
    }

private:
    field _M_field;

    user_config _M_user_config;

    std::mt19937& _M_rand;

    bag_generator _M_bag;
    std::unique_ptr<Iblock_color> _M_color;

    std::unique_ptr<Iattack_table> _M_attack_table;
    std::unique_ptr<Ikick_table> _M_kick_table;
    std::unique_ptr<Ispin_table> _M_spin_table;

    std::optional<tetromino> _M_current, _M_hold;
    std::list<tetromino> _M_queue;
    bool _M_holdable = true;
    // This value can be negative because of mino shape.
    i32 _M_current_x = 0, _M_current_y = 0;
    i32 _M_ghost_x = 0, _M_ghost_y = 0;

    attack_info _M_attack_info = {
        attack_type::SINGLE, 0, -1, spin_type::NONE, false
    };
    // Check if the last input was a spin.
    bool _M_is_last_spin = false;
    // index of test in kick table if the last input was a spin.
    u32 _M_kick_index = 0;

    std::vector<std::string> _M_attack_history;

    struct {
        WINDOW* _M_field = nullptr;
        WINDOW* _M_next = nullptr;
        WINDOW* _M_hold = nullptr;
        WINDOW* _M_stats = nullptr;
        WINDOW* _M_msg = nullptr;
        WINDOW* _M_meta = nullptr;
    } _M_windows;

    std::array<bool, 4> _M_refresh_marked = { false, };
    
    bool _M_running = false;
    time_type _M_start_time, _M_last_fps_time;

    bool _M_restart_req = false;
    // If value is negative, it follows the default restart countdown;
    // otherwise it uses the specified countdown value.
    i32 _M_restart_countdown = -1;

    u32 _M_frame_duration;
    u32 _M_frame_count = 0;

    /* For meta data */
    std::array<std::string_view, 4> _M_meta_data {
        "nothing to undo",
        "nothing to redo",
        "undo buffer full",
        "Perfect Clear!"
    };
    u32 _M_meta_idx = 0;
    // -1 means no meta data is being displayed.
    // Otherwise, it is the index of the meta data being displayed.
    // This value decreases by 1 every frame.
    i32 _M_meta_time = -1;
    bool _M_meta_drawn = false;
    /* ------------- */

    /* For undo/redo */

    struct save_data {
        field _M_field;
        std::optional<tetromino> _M_current;
        std::optional<tetromino> _M_hold;
        std::list<tetromino> _M_queue;
        attack_info _M_attack_info;
        std::optional<std::string> _M_attck_string;
        bag_save_data _M_bag_data;
        std::mt19937 _M_rand;
    };

    buffer<save_data, 20> _M_save_buffer;

    /* ------------- */

    /* For puzzle */

    std::string _M_puzzle_sequence;
    std::vector<tetromino> _M_puzzle_queue;
    bool _M_solved = false;
    u32 _M_solved_count = 0;

    puzzle_function _M_puzzle_func = nullptr;

    /* ---------  */

    inline static constexpr u16 _S_color_interval = 16;
    inline static constexpr u16 _S_color_start = 30;
    inline static constexpr u16 _S_normal_color = _S_color_start;
    inline static constexpr u16 _S_guide_color = _S_color_start + _S_color_interval;
    inline static constexpr u16 _S_locked_color = _S_guide_color + _S_color_interval;
    inline static constexpr u16 _S_gray_color = _S_locked_color + _S_color_interval;

    using rgb_t = std::tuple<u16, u16, u16>;

    // normal = color * (4/5)
    inline static rgb_t _S_normal_coloring(rgb_t __c) {
        auto [__r, __g, __b] = __c;
        __r -= __r / 5;
        __g -= __g / 5;
        __b -= __b / 5;
        return {__r, __g, __b};
    }

    // guide = color * (5/10)
    inline static rgb_t _S_guide_coloring(rgb_t __c) {
        auto [__r, __g, __b] = __c;
        __r = __r / 2;
        __g = __g / 2;
        __b = __b / 2;
        return {__r, __g, __b};
    }
    
    // locked = color
    inline static rgb_t _S_locked_coloring(rgb_t __c)
    { return __c; }

    // gray = grayscale(color)
    // TODO : grayscale issue
    inline static rgb_t _S_gray_coloring(rgb_t __c) {
        auto [__r, __g, __b] = __c;
        u8 __gray = (u8)std::rint(
            std::pow(
                0.2126 * std::pow(__r, 2.2) +
                0.7152 * std::pow(__g, 2.2) +
                0.0722 * std::pow(__b, 2.2),
                1.0 / 2.2
            ) * 76
        );
        return {__gray, __gray, __gray};
    }

    inline static rgb_t _M_convert_base(rgb_t __color, u16 __from, u16 __to) {
        auto [__r, __g, __b] = __color;
        return {
            static_cast<u16>((u32)__r * __to / __from),
            static_cast<u16>((u32)__g * __to / __from),
            static_cast<u16>((u32)__b * __to / __from)
        };
    }

    template <typename _UnaryOp>
    inline static bool _M_init_color(u16 __idx, rgb_t __value, _UnaryOp __op) {
        auto [__r, __g, __b] = _M_convert_base(__op(__value), 255u, 1000u);
        return
            init_color(__idx, __r, __g, __b) == OK &&
            init_pair(__idx, __idx, __idx) == OK;
    }

    void _M_init() {
        _M_frame_duration = 1000 / _M_user_config.game.fps;

        // Setup tables for input


        for (u8 __i = 0; __i < 9u; __i++) {
            auto __color = _M_color->color(static_cast<block_type>(__i));
            _M_init_color(_S_normal_color + __i, __color, _S_normal_coloring);
            _M_init_color(_S_guide_color  + __i, __color, _S_guide_coloring );
            _M_init_color(_S_locked_color + __i, __color, _S_locked_coloring);
            // _M_init_color(_S_gray_color   + __i, __color, _S_gray_coloring  );
        }

        // Use single gray color instead of grayscale.
        _M_init_color(_S_gray_color, {76, 76, 76}, _S_locked_coloring);

        i32 __left_space_width = 30,
            __width = __left_space_width + _M_field.width() * 2 + 4;

        _M_windows._M_field = newwin(
            _M_field.height() + 2, _M_field.width() * 2 + 2,
            1, __left_space_width + 1
        );
        _M_windows._M_next = newwin(
            _M_user_config.next.count * 4 + 2, 10,
            1, _M_field.width() * 2 + __left_space_width + 4
        );
        _M_windows._M_hold = newwin(6, 10, 1, __left_space_width - 10);
        _M_windows._M_stats = newwin(14, __left_space_width, 13, 0);
        _M_windows._M_msg = newwin(1, __width, 0, 0);
        _M_windows._M_meta = newwin(3, __left_space_width, 8, 0);
        
        box(_M_windows._M_field, 0, 0);
        box(_M_windows._M_next, 0, 0);
        box(_M_windows._M_hold, 0, 0);
        box(_M_windows._M_stats, 0, 0);

        mvwprintw(_M_windows._M_next, 0, 3, "Next");
        mvwprintw(_M_windows._M_hold, 0, 3, "Hold");

        _M_refresh_marked = { true, true, true, true };
    }

    tetromino _M_get_next() {
        if (_M_user_config.game.mode == user_config::game_mode::puzzle) {
            if (_M_queue.empty()) {
                if (_M_hold.has_value()) {
                    tetromino __hold = *_M_hold;
                    _M_hold.reset();
                    _M_holdable = false;
                    _M_draw_hold();
                    return __hold;
                } else return tetromino::INVALID;
            } else {
                tetromino __next = _M_queue.front();
                _M_queue.pop_front();
                return __next;
            }
        }

        while (_M_queue.size() <= _M_user_config.next.count)
            _M_queue.push_back(_M_bag.next());

        tetromino __next = _M_queue.front();
        _M_queue.pop_front();
        return __next;
    }

    void _M_draw_mino(
        WINDOW* __win,
        const tetromino& __t,
        u32 __y, u32 __x,
        bool __revision, bool __erase = false, bool __ghost = false
    ) {
        u32 __px = __t.type() == mino_type::O ? 2 : 1,
            __py = __t.type() == mino_type::I ? 1 : 2;
        
        u32 __tx = __revision ? std::min(__t.size(), 3u) : __t.size();
            
        if (!__revision) __px = 0, __py = 0;

        auto __attr =
            __ghost ?
            COLOR_PAIR(_S_guide_color + static_cast<u8>(__t.type())) :
            COLOR_PAIR(_S_normal_color + static_cast<u8>(__t.type()));
        
        for (u32 __j = 0; __j < __t.size(); ++__j) {
            u32 __cx = __px + __x, __cy = __py + __y + __j;
            wmove(__win, __cy, __cx);
            
            for (u32 __k = 0; __k < __tx; ++__k) {
                if (__t.data()[__j][__k] == 0) continue;
                
                wmove(__win, __cy, __cx + __k * 2);
                if (!__erase) wattron(__win, __attr);
                wprintw(__win, "  ");
                if (!__erase) wattroff(__win, __attr);
            }
        }
    }


    // Draw field after tetromino is placed or garbage is added.
    void _M_draw_field(bool __gray = false) {
        wclear(_M_windows._M_field);
        box(_M_windows._M_field, 0, 0);

        const auto& __dt = _M_field.data();

        u32 y = 1;
        auto __it = __dt.rbegin();
        for (; __it != __dt.rend(); ++__it, ++y) {
            wmove(_M_windows._M_field, y, 1);

            for (const auto& [cell, attr] : *__it) {
                if (cell == block_type::EMPTY) {
                    wprintw(_M_windows._M_field, "  ");
                } else {
                    chtype __attr = 0;
                    
                    switch (attr) {
                        case block_attribute::NORMAL:
                            __attr = COLOR_PAIR(
                                (__gray ? _S_gray_color : _S_normal_color +
                                static_cast<u8>(cell)));
                            break;
                        case block_attribute::GUIDE:
                            __attr = COLOR_PAIR(_S_guide_color + static_cast<u8>(cell));
                            break;
                        case block_attribute::LOCKED:
                            __attr = COLOR_PAIR(_S_locked_color + static_cast<u8>(cell));
                            break;
                    }

                    wattron(_M_windows._M_field, __attr);
                    wprintw(_M_windows._M_field, "  ");
                    wattroff(_M_windows._M_field, __attr);
                }
            }
        }

        if (!__gray) {
            _M_draw_ghost_mino();
            _M_draw_current_mino();
        }

        _M_refresh_marked[0] = true;

        _M_draw_stats();
    }

    void _M_draw_next() {
        wclear(_M_windows._M_next);
        box(_M_windows._M_next, 0, 0);

        mvwprintw(_M_windows._M_next, 0, 3, "Next");

        auto iter = _M_queue.begin();
        u32 __size = std::min<u32>(_M_user_config.next.count, _M_queue.size());
        for (u32 __i = 0; __i < __size; ++__i, ++iter)
            _M_draw_mino(_M_windows._M_next, *iter, __i * 4, 1, true);

        _M_refresh_marked[1] = true;
    }

    void _M_draw_hold() {
        wclear(_M_windows._M_hold);
        box(_M_windows._M_hold, 0, 0);

        mvwprintw(_M_windows._M_hold, 0, 3, "Hold");

        if (_M_hold) {
            tetromino __t = *_M_hold;
            
            _M_draw_mino(_M_windows._M_hold, __t, 0, 1, true);
        }

        _M_refresh_marked[2] = true;
    }

    void _M_draw_stats() {
        wclear(_M_windows._M_stats);
        box(_M_windows._M_stats, 0, 0);
        
        mvwprintw(_M_windows._M_stats, 0, 3, "Stats");

        if (_M_user_config.game.mode == user_config::game_mode::puzzle) {
            mvwprintw(_M_windows._M_stats, 2, 1, "Solved count: %d", _M_solved_count);
        } else {

            if (_M_attack_info._M_btb > 0)
                mvwprintw(_M_windows._M_stats, 1, 1, "B2B x%d", _M_attack_info._M_btb);
            else
                mvwprintw(_M_windows._M_stats, 1, 1, "%s", std::string(28, ' ').c_str());

            for (u32 __i = 0, __len = std::min<size_t>(_M_attack_history.size(), 5u); __i < __len; ++__i)
                mvwprintw(_M_windows._M_stats, 2 + __i, 1, "%s",
                    _M_attack_history[_M_attack_history.size() - __i - 1].c_str()
                );
        }

        // Disable clock because of performance issues.
        /*
        auto __elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            clock_type::now() - _M_start_time
        ).count();

        decltype(__elapsed)
            __hours = __elapsed / 3600,
            __minutes = (__elapsed % 3600) / 60,
            __seconds = __elapsed % 60;

        mvwprintw(_M_windows._M_stats, 1, 1, "Time: %s%02ld:%02ld",
            __hours > 0 ? (std::to_string(__hours) + ":").c_str() : "",
            __minutes, __seconds
        );
        */

        _M_refresh_marked[3] = true;
    }

    void _M_draw_current_mino(bool __erase = false) {
        if (!_M_current) return;

        // Draw current tetromino on the field.
        // It will be drawn on the field after it is placed.

        // mino_y = base of collision_down
        _M_draw_mino(
            _M_windows._M_field, *_M_current,
            _M_field.height() - _M_current_y,
            _M_current_x * 2 + 1,
            false,
            __erase
        );


    }

    void _M_draw_ghost_mino(bool __erase = false) {
        if (!_M_current) return;

        if (__erase) {
            _M_draw_mino(
                _M_windows._M_field, *_M_current,
                _M_field.height() - _M_ghost_y,
                _M_ghost_x * 2 + 1,
                false, true, true
            );

            return;
        }

        _M_ghost_x = _M_current_x;
        _M_ghost_y = _M_current_y;

        while (!_M_is_in_collision(_M_ghost_x, _M_ghost_y - 1, *_M_current))
            _M_ghost_y -= 1;

        _M_draw_mino(
            _M_windows._M_field, *_M_current,
            _M_field.height() - _M_ghost_y,
            _M_ghost_x * 2 + 1,
            false, false, true
        );
    }

    void _M_draw_all() {
        _M_draw_field();
        _M_draw_next();
        _M_draw_hold();
        _M_draw_stats();
    }

    bool _M_is_in_collision(
        i32 __x, i32 __y, const tetromino& __t
    ) {
        if (__t == tetromino::INVALID) return true;

        for (u32 __i = 0; __i < __t.size(); ++__i) {
            for (u32 __j = 0; __j < __t.size(); ++__j) {
                if (__t.data()[__i][__j] == 0) continue;

                i32 __px = __x + __j,
                    __py = __y - __i;

                block_type __bt = _M_field.get_block(__px, __py);
                if (__bt != block_type::EMPTY) return true;
            }
        }

        return false;
    }

    /*
    bool _M_is_in_collision() {
        if (!_M_current) return true;

        return _M_collision_check(
            _M_current_x, _M_current_y, *_M_current
        );
    }
    */

    // For spin check.
    bool _M_is_immobile() {
        return 
            _M_is_in_collision(_M_current_x - 1, _M_current_y, *_M_current) &&
            _M_is_in_collision(_M_current_x + 1, _M_current_y, *_M_current) &&
            _M_is_in_collision(_M_current_x, _M_current_y + 1, *_M_current) &&
            _M_is_in_collision(_M_current_x, _M_current_y - 1, *_M_current);
    }

    bool _M_down_once() {
        bool __cur = _M_user_config.control.inf_soft_drop;
        _M_user_config.control.inf_soft_drop = false;

        bool __r = down();

        _M_user_config.control.inf_soft_drop = __cur;

        return __r;
    }

    void _M_start(u32 __countdown) {
        _M_queue = std::list<tetromino>();

        if (_M_user_config.game.mode == user_config::game_mode::puzzle) {
            if (_M_puzzle_func == nullptr)
                throw std::runtime_error("Puzzle function is not set.");
            if (_M_puzzle_sequence.empty())
                throw std::runtime_error("Puzzle sequence is not set.");
            
            if (_M_puzzle_queue.empty())
                _M_puzzle_queue = *tetromino::gen(_M_puzzle_sequence, _M_rand);
            _M_queue = std::list<tetromino>(_M_puzzle_queue.begin(), _M_puzzle_queue.end());

            _M_restart_countdown = 0;
        } else {
            while (_M_queue.size() < _M_user_config.next.count)
                _M_queue.push_back(_M_bag.next());
        }
        
        _M_hold = std::nullopt;
        _M_holdable = _M_user_config.hold.enabled;

        _M_draw_all();
        wnoutrefresh(_M_windows._M_field);
        wnoutrefresh(_M_windows._M_next);
        wnoutrefresh(_M_windows._M_hold);
        wnoutrefresh(_M_windows._M_stats);
        doupdate();

        if (__countdown > 0) {
            u32 __bw, __bh;
            getbegyx(_M_windows._M_field, __bh, __bw);

            u32 __w = __bw + converter::center(_M_field.width() * 2 + 2, 6),
                __h = __bh + converter::center(_M_field.height() + 2, 4);
            
            WINDOW* __pw = newwin(5, 7, __h, __w);

            __countdown = std::min(9u, __countdown);
            for (; __countdown > 0; __countdown--) {
                mvwprintw(__pw, 0, 0, "%s", converter::i2a[__countdown].data());
                wrefresh(__pw);

                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }

            delwin(__pw);
        }

        _M_start_time = clock_type::now();
        _M_last_fps_time = _M_start_time;

        flushinp();
        spawn();
        _M_draw_all();
        _M_running = true;
    }

    void _M_reset_meta() { _M_meta_time = 0; }

    template <typename Rep2, typename Period2>
    void _M_set_meta(u32 __idx, std::chrono::duration<Rep2, Period2> __time) {
        _M_meta_idx = __idx;
        _M_meta_time =
            std::chrono::duration_cast<std::chrono::seconds>(__time).count() *
            _M_user_config.game.fps;
        _M_meta_drawn = false;
    }

public:
    bool left() {
        if (_M_is_in_collision(_M_current_x - 1, _M_current_y, *_M_current)) return false;

        _M_draw_current_mino(true);
        _M_draw_ghost_mino(true);

        _M_current_x -= 1;

        _M_draw_ghost_mino();

        _M_draw_current_mino();
        _M_refresh_marked[0] = true;

        _M_is_last_spin = false;

        return true;
    }

    bool right() {
        if (_M_is_in_collision(_M_current_x + 1, _M_current_y, *_M_current)) return false;

        _M_draw_current_mino(true);
        _M_draw_ghost_mino(true);

        _M_current_x += 1;

        _M_draw_ghost_mino();

        _M_draw_current_mino();
        _M_refresh_marked[0] = true;

        _M_is_last_spin = false;

        return true;
    }

    bool down() {
        if (_M_is_in_collision(_M_current_x, _M_current_y - 1, *_M_current)) return false;

        if (_M_user_config.control.inf_soft_drop) {
            _M_draw_current_mino(true);

            while (!_M_is_in_collision(_M_current_x, _M_current_y - 1, *_M_current))
                _M_current_y -= 1;
            
        } else {
            _M_draw_current_mino(true);

            _M_current_y -= 1;
        }

        _M_is_last_spin = false;

        _M_draw_current_mino();
        _M_refresh_marked[0] = true;

        return true;
    }

    void rotate(rotation __r) {
        if (!_M_current) return;
        
        tetromino __t = *_M_current;
        __t.rotate(__r);

        const auto& __table =
            _M_kick_table->get(__t, _M_current->direction(), __t.direction());
        
        bool __flag = false;
        i32 __idx = -1;
        std::pair<i32, i32> __p = { 0, 0 };
        auto& [__px, __py] = __p;
        do {
            if (__idx != -1)
                __p = __table[__idx];

            if (!_M_is_in_collision(_M_current_x + __px, _M_current_y + __py, __t))
            { __flag = true; break; }

            __idx++;
        } while ((u32)__idx < __table.size());

        if (!__flag) return;

        _M_draw_current_mino(true);
        _M_draw_ghost_mino(true);

        _M_current = __t;
        _M_current_x += __px;
        _M_current_y += __py;

        _M_is_last_spin = true;
        _M_kick_index = __idx;

        _M_draw_ghost_mino();
        _M_draw_current_mino();
        _M_refresh_marked[0] = true;
    }

    void drop() {
        if (!_M_current) return;

        _M_draw_current_mino(true);
        _M_draw_ghost_mino(true);

        while (!_M_is_in_collision(_M_current_x, _M_current_y - 1, *_M_current)) {
            _M_current_y -= 1;
            _M_is_last_spin = false;
        }

        bool __imm = _M_is_immobile();

        _M_field.put_mino(_M_current_x, _M_current_y, *_M_current);

        spin_type __sp =
            _M_is_last_spin ?
            _M_spin_table->get({
                *_M_current, _M_current_x, _M_current_y,
                _M_kick_index, __imm,
                _M_field
            }) : spin_type::NONE;

        u32 __lines = _M_field.proceed_lines();

        if (__lines > 0) {
            // Check perfect clear
            _M_attack_info._M_pc = _M_field.is_empty();
            _M_attack_info._M_type = static_cast<attack_type>(__lines - 1);
            _M_attack_info._M_combo++;
            _M_attack_info._M_spin = __sp;
            if (_M_attack_info._M_spin == spin_type::NONE && 
                _M_attack_info._M_type != attack_type::QUAD)
                _M_attack_info._M_btb = -1;
            else _M_attack_info._M_btb++;
            
            std::stringstream ss;
            ss << "[" << _M_attack_table->get(_M_attack_info) << "] "
               << _M_attack_info.to_string(_M_current->to_char());
            _M_attack_history.push_back(ss.str());
            _M_save_buffer.current()._M_attck_string = ss.str();

            if (_M_attack_info._M_pc)
                _M_set_meta(3, std::chrono::seconds(2));
        } else {
            _M_attack_info._M_pc = false;
            _M_attack_info._M_type = attack_type::SINGLE;
            _M_attack_info._M_combo = 0;
            _M_attack_info._M_spin = spin_type::NONE;
        }

        if (_M_user_config.game.mode == user_config::game_mode::puzzle) {
            if (_M_puzzle_func) {
                if (_M_puzzle_func(
                    _M_field, *_M_current, _M_current_x, _M_current_y,
                    _M_attack_info
                )) _M_solved = true;
            }
        }

        // Draw in spawn.
        spawn();
        _M_draw_field();
    }

    void spawn(bool __new = true, bool __hold = false) {
        _M_draw_ghost_mino(true);

        if (__new)
            _M_current = _M_get_next();

        if (_M_user_config.game.mode == user_config::game_mode::puzzle) {
            if (_M_current == tetromino::INVALID) {
                if (_M_solved) {
                    _M_puzzle_queue = *tetromino::gen(_M_puzzle_sequence, _M_rand);

                    _M_solved_count++;
                    _M_solved = false;
                    _M_draw_stats();
                }

                _M_queue = std::list<tetromino>(
                    _M_puzzle_queue.begin(), _M_puzzle_queue.end()
                );
                
                _M_restart_req = true;
                return;
            }
        }

        _M_current_x = _M_field.width() / 2 - (_M_current->size() + 1) / 2;
        _M_current_y = _M_user_config.spawn.base_height;

        if (_M_is_in_collision(_M_current_x, _M_current_y, *_M_current)) {
            if (_M_user_config.spawn.extended) {
                u32 __i = 0;
                for (; __i < _M_user_config.spawn.extended_height; ++__i) {
                    if (!_M_is_in_collision(_M_current_x, _M_current_y + __i, *_M_current))
                        break;
                }

                if (__i < _M_user_config.spawn.extended_height) {
                    _M_current_y += __i;
                } else { gameover(); return; }
            } else { gameover(); return; }
        }

        if (!__hold)
            _M_holdable = true;

        _M_draw_ghost_mino();
        _M_draw_current_mino();
        _M_refresh_marked[0] = true;

        if (!__hold) {
            save_data __dt;

            __dt._M_field = _M_field;
            __dt._M_current = _M_current;
            __dt._M_hold = _M_hold;
            __dt._M_queue = _M_queue;
            __dt._M_attack_info = _M_attack_info;
            __dt._M_attck_string = std::nullopt;
            __dt._M_bag_data = _M_bag.save();
            __dt._M_rand = _M_rand;

            _M_save_buffer.push(std::move(__dt));
        }

        _M_draw_next();
    }

    void hold() {
        if (!_M_user_config.hold.enabled) return;
        if (!_M_holdable) return;

        if (_M_hold) {
            swap(_M_current, _M_hold);
            spawn(false, true);
        }
        else {
            if (_M_queue.empty()) return;

            _M_hold = _M_current;
            spawn(true);
        }

        _M_hold->set_direction(0);
        
        if (!_M_user_config.hold.infinite)
            _M_holdable = false;

        _M_draw_field();
        _M_draw_hold();
    }

    void proceed_input(i32 ch) {
        if (!_M_running) return;
            
        if (ch == ERR) return;
        if (ch == KEY_RESIZE) {
            clear(); _M_draw_all();
            return;
        }

        ch = std::tolower(ch);

#ifdef DEBUG
        if (ch == 'g')  {
            // Debugging: move down once
            _M_down_once();
            return;
        }
#endif
        if (!_M_user_config.control.key_map.contains(ch)) return;
            
        switch (_M_user_config.control.key_map.at(ch)) {
            case control_key::LEFT: left(); break;
            case control_key::RIGHT: right(); break;
            case control_key::DOWN: down(); break;
            case control_key::ROTATE_CW: rotate(rotation::cw); break;
            case control_key::ROTATE_CCW: rotate(rotation::ccw); break;
            case control_key::ROTATE_180: rotate(rotation::_180); break;
            case control_key::DROP: drop(); break;
            case control_key::HOLD: hold(); break;
            case control_key::RESET: _M_restart_req = true; return;
            case control_key::QUIT: gameover(); break;
            case control_key::UNDO: undo(); break;
            case control_key::REDO: redo(); break;
            default: break;
        }
    }

    void garbage(u32 __cnt, i32 __hole = -1) {
        _M_field.put_garbage(__cnt, __hole);
        _M_draw_field();
    }

    void start() { _M_start(_M_user_config.game.start_countdown); }

    void restart() {
        reset();

        u32 __countdown = _M_restart_countdown < 0 ?
            _M_user_config.game.restart_countdown :
            _M_restart_countdown;
        _M_start(__countdown);
    }

    void gameover() {
        _M_draw_field(true);
        _M_draw_next();
        refresh();

        _M_running = false;

        mvwprintw(_M_windows._M_msg, 0, 0, "Game Over! Press any key to exit...");
        wnoutrefresh(_M_windows._M_msg);
    }

    void reset() {
        _M_restart_req = false;
        _M_running = false;

        _M_field.clear();
        _M_bag.reset();
        
        _M_queue = std::list<tetromino>();
        _M_current = std::nullopt;
        _M_hold = std::nullopt;

        _M_attack_info = {
            attack_type::SINGLE, 0, -1, spin_type::NONE, false
        };
        _M_is_last_spin = false;

        _M_attack_history.clear();
    }

    void undo() {
        if (!_M_save_buffer.prev()) {
            _M_set_meta(0, std::chrono::seconds(3));
            return;
        }

        const save_data& __dt = _M_save_buffer.current();

        _M_field = __dt._M_field;
        _M_current = __dt._M_current;
        _M_hold = __dt._M_hold;
        _M_queue = __dt._M_queue;
        _M_attack_info = __dt._M_attack_info;
        if (__dt._M_attck_string.has_value())
            _M_attack_history.pop_back();
        _M_bag.load(__dt._M_bag_data);
        _M_rand = __dt._M_rand;

        _M_current_x = _M_field.width() / 2 - (_M_current->size() + 1) / 2;
        _M_current_y = _M_user_config.spawn.base_height;

        _M_reset_meta();

        _M_draw_all();
    }

    void redo() {
        std::string __atk =
            _M_save_buffer.current()._M_attck_string.has_value() ?
            *_M_save_buffer.current()._M_attck_string : "";

        if (!_M_save_buffer.next()) {
            _M_set_meta(1, std::chrono::seconds(3));
            return;
        }

        const save_data& __dt = _M_save_buffer.current();

        _M_field = __dt._M_field;
        _M_current = __dt._M_current;
        _M_hold = __dt._M_hold;
        _M_queue = __dt._M_queue;
        _M_attack_info = __dt._M_attack_info;
        if (!__atk.empty())
            _M_attack_history.push_back(__atk);
        _M_bag.load(__dt._M_bag_data);
        _M_rand = __dt._M_rand;

        _M_current_x = _M_field.width() / 2 - (_M_current->size() + 1) / 2;
        _M_current_y = _M_user_config.spawn.base_height;

        _M_reset_meta();

        _M_draw_all();
    }

    bool restart_requested() const { return _M_restart_req; }
    bool is_running() const { return _M_running; }
    u32 frame_duration() const { return _M_frame_duration; }

    u32 get_fps() {
        u32 __rate = _M_frame_count;

        _M_frame_count = 0;

        return std::min(999u, __rate);
    }

    void refresh() {
        if (!_M_running) return;

        for (i32 __i = 0; __i <  4; ++__i) {
            if (_M_refresh_marked[__i]) {
                switch (__i) {
                    case 0: wnoutrefresh(_M_windows._M_field); break;
                    case 1: wnoutrefresh(_M_windows._M_next);  break;
                    case 2: wnoutrefresh(_M_windows._M_hold);  break;
                    case 3: wnoutrefresh(_M_windows._M_stats); break;
                }
                _M_refresh_marked[__i] = false;
            }
        }

        if (_M_meta_time > 0) {
            if (!_M_meta_drawn) {
                _M_meta_drawn = true;
                box(_M_windows._M_meta, 0, 0);
                mvwprintw(_M_windows._M_meta, 1, 1, "%s", 
                    _M_meta_data[_M_meta_idx].data()
                );
                wnoutrefresh(_M_windows._M_meta);
            }

            _M_meta_time--;
        } else if (_M_meta_time == 0) {
            werase(_M_windows._M_meta);
            wnoutrefresh(_M_windows._M_meta);
        }

        auto now = clock_type::now();
        if (now - _M_last_fps_time >= std::chrono::seconds(1)) {
            _M_last_fps_time = now;

            u32 fps = get_fps();

            mvwprintw(_M_windows._M_msg, 0, 0, "FPS: %3d", fps);
            wnoutrefresh(_M_windows._M_msg);
        }

        doupdate();

        _M_frame_count++;
    }

    void set_puzzle_function(puzzle_function __func) { _M_puzzle_func = __func; }
    void set_puzzle_sequence(const std::string& __seq) {
        _M_puzzle_sequence = __seq;

        auto __test = tetromino::gen(__seq, _M_rand);

        if (!__test)
            throw std::runtime_error("Invalid puzzle sequence: " + __seq);
    }
};