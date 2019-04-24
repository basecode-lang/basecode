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

#include "term_stream_builder.h"

namespace basecode::common {

    std::string color_code(term_colors_t fg_color, term_colors_t bg_color) {
        return fmt::format("\033[1;{};{}m", (uint32_t) fg_color, ((uint32_t) bg_color) + 10);
    }

    ///////////////////////////////////////////////////////////////////////////

    term_stream::~term_stream() {
    }

    ///////////////////////////////////////////////////////////////////////////

    term_stream_builder::term_stream_builder(bool enabled) : _enabled(enabled) {
    }

    bool term_stream_builder::enabled() const {
        return _enabled;
    }

    std::string term_stream_builder::colorize(
            const std::string& text,
            term_colors_t fg_color,
            term_colors_t bg_color) {
        if (!_enabled)
            return text;
        return fmt::format(
            "{}{}{}",
            color_code(fg_color, bg_color),
            text,
            color_code_reset());
    }

    std::string term_stream_builder::colorize_range(
            const std::string& text,
            size_t begin,
            size_t end,
            term_colors_t fg_color,
            term_colors_t bg_color) {
        if (!_enabled)
            return text;
        std::stringstream colored_source;
        for (size_t j = 0; j < text.length(); j++) {
            if (begin == end && j == begin) {
                colored_source << color_code(fg_color, bg_color);
                colored_source << text[j];
                colored_source << color_code_reset();
            } else {
                if (j == begin) {
                    colored_source << color_code(fg_color, bg_color);
                } else if (j == end) {
                    colored_source << color_code_reset();
                }
                colored_source << text[j];
            }
        }
        return colored_source.str();
    }

    term_stream_unique_ptr term_stream_builder::use_stream(std::stringstream& stream) const {
        if (_enabled)
            return term_stream_unique_ptr(new ansi_stream(stream));
        else
            return term_stream_unique_ptr(new ascii_stream(stream));
    }

}