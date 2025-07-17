#include <iostream>

#include <random>

#include <ncurses.h>

#include <lib/intdef>
#include <rules/tetromino.hpp>
#include <rules/bag.hpp>
#include <rules/attack_table.hpp>
#include <rules/field.hpp>

#include <game.hpp>
#include <env.hpp>

bool init() {
    if (initscr() == nullptr) return false;
    if (start_color() == ERR) return false;
    if (curs_set(0) == ERR) return false;
    if (noecho() == ERR) return false;
    if (cbreak() == ERR) return false;
    if (keypad(stdscr, TRUE) == ERR) return false;
    if (use_default_colors() == ERR) return false;
    
    nodelay(stdscr, TRUE);

    return true;
}

int main(int argc, char** argv) {
    env::initialize(argc, argv);

    if (!init()) {
        std::cerr << "Failed to initialize ncurses.\n";
        return 1;
    }

    refresh();

    user_config __config;
    __config.hold.infinite = true;
    __config.control.inf_soft_drop = true;
    __config.game.fps = 120;
    __config.game.start_countdown = 0;
    // __config.game.mode = user_config::game_mode::puzzle;

    std::mt19937 engine(std::random_device{}());
    game g(engine, __config);

    // For puzzle mode.
    /*
    g.set_puzzle_function([](
        const field& f,
        tetromino t,
        i32 x, i32 y,
        attack_info atk
    ) -> bool {
        // Example puzzle function: Check if the field is empty.
        return atk._M_pc;
    });
    g.set_puzzle_sequence("*p4*!");
    */
    
    g.start();
    refresh();

    while (true) {
        if (!g.is_running()) break;

        i32 ch = getch();
        g.proceed_input(ch);

        if (g.restart_requested())
            g.restart();

        g.refresh();

        std::this_thread::sleep_for(std::chrono::milliseconds(g.frame_duration()));
    }

    refresh();
    flushinp();
    while (getch() == ERR);

    endwin();
}