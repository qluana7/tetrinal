#include <iostream>
#include <fstream>

#include <vector>
#include <string>

#include <random>
#include <atomic>

#include <intdef>
#include <strlib>

#include <ncurses.h>

#include <rules/tetromino.hpp>
#include <rules/bag.hpp>
#include <rules/attack_table.hpp>
#include <rules/field.hpp>
#include <game.hpp>

std::string mino2str(tetromino mino)
{ return std::string(1, (char)mino); }

void print_mino(tetromino mino) {
    std::cout << "Mino type: " << static_cast<u32>(mino.type()) << "\n";
    std::cout << "Mino direction: " << mino.direction() << "\n";
    std::cout << "Mino size: " << mino.size() << "\n";
    std::cout << "Mino data:\n";
    for (const auto& row : mino.data()) {
        for (const auto& cell : row) {
            std::cout << static_cast<int>(cell) << " ";
        }
        std::cout << "\n";
    }
    std::cout << "Mino collision: "
              << "left=" << mino.collision().left
              << ", right=" << mino.collision().right
              << ", down=" << mino.collision().down
              << ", up=" << mino.collision().up
              << "\n";
}

constexpr int FRAME_RATE = 60; // Frames per second
constexpr int FRAME_DURATION = 1000 / FRAME_RATE; // Milliseconds per frame
std::atomic<int> __frame_count;

bool init() {
    if (initscr() == nullptr) return false;
    if (start_color() == ERR) return false;
    if (curs_set(0) == ERR) return false;
    if (noecho() == ERR) return false;
    if (cbreak() == ERR) return false;
    if (keypad(stdscr, TRUE) == ERR) return false;
    if (use_default_colors() == ERR) return false;
    
    nodelay(stdscr, TRUE);
    __frame_count.store(0);

    return true;
}

int main() {
    if (!init()) {
        std::cerr << "Failed to initialize ncurses.\n";
        return 1;
    }

    refresh();
    user_config __config;
    __config.hold.infinite = true;
    __config.control.inf_soft_drop = true;
    game<
        block_color::bright,
        attack_tables::tetrio,
        kick_tables::srs_plus,
        spin_tables::all_mini_plus
    > g(__config);

    /*
    std::jthread __fps_thread([](std::stop_token __token) {
        while (true) {
            if (__token.stop_requested())
                break;

            int frame_count = __frame_count.load();
            mvprintw(0, 0, "FPS: %2d", frame_count);
            __frame_count.store(0);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
    */
    
    g.start();

    while (true) {
        if (!g.is_running()) break;

        if (g.restart_requested())
            g.restart();

        g.refresh();

        std::this_thread::sleep_for(std::chrono::milliseconds(FRAME_DURATION));
        // napms(FRAME_DURATION);
    }
    
    refresh();
    while (getch() == ERR);
    while (getch() != ERR);

    // __fps_thread.request_stop();
    // __fps_thread.join();

    endwin();
}