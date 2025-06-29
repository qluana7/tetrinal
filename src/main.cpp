#include <iostream>

#include <random>

#include <intdef>

#include <ncurses.h>

#include <rules/tetromino.hpp>
#include <rules/bag.hpp>
#include <rules/attack_table.hpp>
#include <rules/field.hpp>
#include <game.hpp>

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

int main() {
    if (!init()) {
        std::cerr << "Failed to initialize ncurses.\n";
        return 1;
    }

    user_config __config;
    __config.hold.infinite = true;
    __config.control.inf_soft_drop = true;
    __config.game.fps = 120;
    __config.game.start_countdown = 9;

    std::mt19937 engine;
    game g(engine, __config);
    
    g.start();
    refresh();

    while (true) {
        if (!g.is_running()) break;

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