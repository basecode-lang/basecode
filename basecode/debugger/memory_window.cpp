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

#include <fmt/format.h>
#include <compiler/session.h>
#include "environment.h"
#include "memory_window.h"

namespace basecode::debugger {

    memory_window::memory_window(
            window* parent,
            int x,
            int y,
            int width,
            int height) : window(parent,
                                 x,
                                 y,
                                 width,
                                 height,
                                 "Memory") {
    }

    void memory_window::on_draw(environment& env) {
        auto& terp = env.session().terp();
        auto address = reinterpret_cast<uint64_t>(terp.heap() + row());

        auto page_height = max_height() - 1;
        auto page_width = max_width() - 2;

        for (int row = 1; row < page_height; row++) {
                auto start_address = address;
                std::stringstream bytes_stream;
                std::stringstream chars_stream;
            mvwhline(ptr(), row, 1, ' ', page_width);

            for (size_t x = 0; x < 8; x++) {
                auto value = terp.read(vm::op_sizes::byte, address);
                bytes_stream << fmt::format("{:02X} ", value);
                chars_stream << (char)(isprint(static_cast<int>(value)) ? value : '.');
                address++;
            }

            auto value = fmt::format(
                "${:016X}: {}{}",
                start_address,
                bytes_stream.str(),
                chars_stream.str());
            size_t clip_length = value.length();
            if (clip_length > page_width)
                clip_length = static_cast<size_t>(page_width);

            auto column_offset = column();
            if (column_offset > 0
            &&  clip_length > column_offset) {
                clip_length -= column_offset;
            }
            value = value.substr(0, clip_length);

            mvwprintw(ptr(), row, 1, "%s", value.c_str());
        }
    }

};