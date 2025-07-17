#pragma once

#include <array>

#include <string_view>

#include <lib/intdef>

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

    /**
     * Calculate the center position for a given width and size.
     * 
     * @param __d The total width of the area.
     * @param __s The size of the element to be centered.
     * 
     * @return The starting position to center the element within the area.
     */
    static u32 center(u32 __d, u32 __s) {
        return (__d > __s) ? (__d - __s) / 2 : 0;
    }

    /**
     * Calculate the starting position to place an element of size `__s` within a total width `__d`,
     * such that the element's left edge is positioned at the `__p` percent location of the available space.
     * 
     * For example, if `__p` is 0, the element is left-aligned; if `__p` is 50, the element is centered;
     * if `__p` is 100, the element is right-aligned (if space allows).
     * 
     * @param __d The total width of the area.
     * @param __s The size of the element to be positioned.
     * @param __p The percentage (0~100) indicating the relative position within the available space.
     * 
     * @return The starting position to place the element at the specified percentage location.
     */
    static u32 to_ratio(u32 __d, u32 __s, u32 __p) {
        if (__d <= __s) return 0; // If the total width is less than or equal to the size, return 0.
        if (__p > 100) __p = 100; // Clamp percentage to a maximum of 100.
        if (__p < 0) __p = 0; // Clamp percentage to a minimum of 0.

        return (__d - __s) * __p / 100;
    }
};