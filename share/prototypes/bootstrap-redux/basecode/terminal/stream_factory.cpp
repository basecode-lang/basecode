// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
//       C O M P I L E R  P R O J E C T
//
// Copyright (C) 2019 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include "stream_factory.h"

namespace basecode::terminal {

    adt::string_t color_code(colors_t fg_color, colors_t bg_color) {
        return format::format(
            "\033[1;{};{}m",
            (uint32_t) fg_color,
            ((uint32_t) bg_color) + 10);
    }

    ///////////////////////////////////////////////////////////////////////////

    adt::string_t stream_factory_t::colorize(
            std::string_view text,
            colors_t fg_color,
            colors_t bg_color) const {
        if (!_enabled)
            return text;
        return format::format(
            "{}{}{}",
            color_code(fg_color, bg_color),
            text,
            color_code_reset());
    }

    bool stream_factory_t::enabled() const {
        return _enabled;
    }

    void stream_factory_t::enabled(bool value) {
        _enabled = value;
    }

    adt::string_t stream_factory_t::colorize_range(
            utf8::reader_t& reader,
            size_t begin,
            size_t end,
            colors_t fg_color,
            colors_t bg_color) const {
        format::memory_buffer_t buffer{};

        if (!_enabled) {
            format::format_to(buffer, "{}", reader.slice());
            return format::to_string(buffer);
        }

        auto j = 0;
        result_t r{};
        auto in_colored_range = false;
        while (true) {
            auto rune = reader.next(r);
            if (rune == utf8::rune_eof)
                break;

            if (begin == end && j == begin) {
                format::format_to(
                    buffer,
                    "{}{}{}",
                    color_code(fg_color, bg_color),
                    rune,
                    color_code_reset());
            } else {
                if (j == begin) {
                    format::format_to(buffer, "{}", color_code(fg_color, bg_color));
                    in_colored_range = true;
                } else if (j == end) {
                    format::format_to(buffer, "{}", color_code_reset());
                    in_colored_range = false;
                }
                format::format_to(buffer, "{}", rune);
            }

            j++;
        }

        if (in_colored_range) {
            format::format_to(buffer, "{}", color_code_reset());
        }

        return format::to_string(buffer);
    }

    stream_unique_ptr_t stream_factory_t::use_memory_buffer(format::memory_buffer_t& buffer) const {
        if (_enabled)
            return stream_unique_ptr_t(new ansi_stream_t(buffer));
        else
            return stream_unique_ptr_t(new ascii_stream_t(buffer));
    }

}