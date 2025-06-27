#pragma once

#include <array>

#include <string_view>

#include <intdef>

/* static */ class converter {
public:
    // Convert 0 ~ 9 numbers to ascii image.
    // Each number is represented by a 6x4 grid of characters.
    constexpr static std::array<std::string_view, 10> i2a = {
        R"(  __  )""\n"
        R"( /  \ )""\n"
        R"(| () |)""\n"
        R"( \__/ )""\n",
        R"(  _   )""\n"
        R"( / |  )""\n"
        R"( | |  )""\n"
        R"( |_|  )""\n",
        R"( ___  )""\n"
        R"(|_  ) )""\n"
        R"( / /  )""\n"
        R"(/___| )""\n",
        R"( ____ )""\n"
        R"(|__ / )""\n"
        R"( |_ \ )""\n"
        R"(|___/ )""\n",
        R"( _ _  )""\n"
        R"(| | | )""\n"
        R"(|_  _|)""\n"
        R"(  |_| )""\n",
        R"( ___  )""\n"
        R"(| __| )""\n"
        R"(|__ \ )""\n"
        R"(|___/ )""\n",
        R"(  __  )""\n"
        R"( / /  )""\n"
        R"(/ _ \ )""\n"
        R"(\___/ )""\n",
        R"( ____ )""\n"
        R"(|__  |)""\n"
        R"(  / / )""\n"
        R"( /_/  )""\n",
        R"( ___  )""\n"
        R"(( _ ) )""\n"
        R"(/ _ \ )""\n"
        R"(\___/ )""\n",
        R"( ___  )""\n"
        R"(/ _ \ )""\n"
        R"(\_, / )""\n"
        R"( /_/  )""\n"
    };

    static u32 center(u32 __d, u32 __s) {
        return (__d > __s) ? (__d - __s) / 2 : 0;
    }
};