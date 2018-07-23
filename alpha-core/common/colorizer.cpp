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

#include <sstream>
#include <fmt/format.h>
#include "colorizer.h"

namespace basecode::common {

    std::string colorizer::colorize(
            const std::string& text,
            term_colors_t color) {
        return fmt::format("\033[1;{}m{}\033[0m", (uint32_t)color, text);
    }

    std::string colorizer::colorize_range(
            const std::string& text,
            size_t begin,
            size_t end,
            term_colors_t color) {
        std::stringstream colored_source;
        for (size_t j = 0; j < text.length(); j++) {
            if (j == begin) {
                colored_source << fmt::format(
                    "\033[1;{}m",
                    (uint32_t)color);
            } else if (j == end) {
                colored_source << "\033[0m";
            }
            colored_source << text[j];
        }
        return colored_source.str();
    }

};