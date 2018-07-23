// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <string>
#include <cstdint>

namespace basecode::common {

//    foreground background
//    black        30         40
//    red          31         41
//    green        32         42
//    yellow       33         43
//    blue         34         44
//    magenta      35         45
//    cyan         36         46
//    white        37         47

    enum class term_colors_t : uint8_t {
        black = 30,
        red,
        green,
        yellow,
        blue,
        magenta,
        cyan,
        white
    };

    class colorizer {
    public:
        static std::string colorize(
            const std::string& text,
            term_colors_t color);

        static std::string colorize_range(
            const std::string& text,
            size_t begin,
            size_t end,
            term_colors_t color);
    };

};

