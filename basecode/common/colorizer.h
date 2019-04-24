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

    enum class term_colors_t : uint8_t {
        black = 30,
        red,
        green,
        yellow,
        blue,
        magenta,
        cyan,
        light_gray,
        default_color = 39,

        dark_gray = 90,
        light_red,
        light_green,
        light_yellow,
        light_blue,
        light_magenta,
        light_cyan,
        white
    };

    // N.B. this is not thread safe
    extern bool g_color_enabled;

    class colorizer {
    public:
        static std::string colorize(
            const std::string& text,
            term_colors_t fg_color,
            term_colors_t bg_color = term_colors_t::default_color);

        static std::string colorize_range(
            const std::string& text,
            size_t begin,
            size_t end,
            term_colors_t fg_color,
            term_colors_t bg_color = term_colors_t::default_color);

        static constexpr const char* color_code_reset();

        static std::string color_code_fg(term_colors_t color);

        static std::string color_code_bg(term_colors_t color);

        static std::string color_code(term_colors_t fg_color, term_colors_t bg_color);
    };

}

