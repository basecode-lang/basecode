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
        white
    };

    class colorizer {
    public:
        static std::string colorize(
            const std::string& text,
            term_colors_t fg_color,
            term_colors_t bg_color = term_colors_t::black);

        static std::string colorize_range(
            const std::string& text,
            size_t begin,
            size_t end,
            term_colors_t fg_color,
            term_colors_t bg_color = term_colors_t::black);

        static constexpr const char* color_code_reset();

        static std::string color_code(term_colors_t color);

        static constexpr term_colors_t make_bg_color(term_colors_t color);
    };

};

