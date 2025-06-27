#pragma once

#include <map>

#include <intdef>

// configuration about game.
struct user_config {
    struct hold_config {
        bool enabled = true;
        bool infinite = false;
    } hold;

    struct next_config {
        bool enabled = true;
        u32 count = 5;
    } next;

    struct field_config {
        // needs extra height for spawn mino and garbage.
        // default extra height is 4.
        u32 width = 10, height = 20, extra_height = 4;
    } field;

    struct spawn_config {
        u32 base_height = 21;
        bool extended = false;
        u32 extended_height = 1;
    } spawn;

    struct control_config {
        enum class KEYS : i32 {
            LEFT, RIGHT, DOWN, ROTATE_CW, ROTATE_CCW, ROTATE_180,
            DROP, HOLD, RESET, QUIT, UNDO, REDO
        };
        
        // Map of control keys.
        std::map<i32, KEYS> key_map = {
            { KEY_LEFT,  KEYS::LEFT       },
            { KEY_RIGHT, KEYS::RIGHT      },
            { KEY_DOWN,  KEYS::DOWN       },
            { KEY_UP,    KEYS::ROTATE_CW  },
            { 'z',       KEYS::ROTATE_CCW },
            { 'a',       KEYS::ROTATE_180 },
            { ' ',       KEYS::DROP       },
            { 'c',       KEYS::HOLD       },
            { 'r',       KEYS::RESET      },
            // ESC in ascii = 27
            { 27,        KEYS::QUIT       },
            { 'u',       KEYS::UNDO       },
            { 'i',       KEYS::REDO       }
        };

        bool inf_soft_drop = true;
    } control;

    struct game_config {
        u32 fps = 60;
    } game;

    // TODO : Add more config options as needed
};