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

#include <vm/terp.h>
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

    uint64_t memory_window::address() const {
        return _address;
    }

    void memory_window::address(uint64_t value) {
        if (_address != value) {
            row(0);
            column(0);
            _address = value;
            mark_dirty();
        }
    }

    void memory_window::on_draw(environment& env) {
        auto& terp = env.session().terp();
        auto end_of_heap = terp.heap_vector(vm::heap_vectors_t::top_of_stack);
        auto address = reinterpret_cast<uint64_t>(_address + row());

        auto page_height = static_cast<size_t>(max_height() - 1);
        auto page_width = static_cast<size_t>(max_width() - 2);

        for (size_t row = 1; row < page_height; row++) {
            auto start_address = address;
            std::stringstream bytes_stream;
            std::stringstream chars_stream;

            mvwhline(ptr(), row, 1, ' ', page_width);

            for (size_t x = 0; x < 8; x++) {
                if (address < end_of_heap) {
                    auto value = terp.read(vm::op_sizes::byte, address);
                    bytes_stream << fmt::format("{:02X} ", value);
                    chars_stream << (char) (isprint(static_cast<int>(value)) ? value : '.');
                    address++;
                }
            }

            auto value = fmt::format(
                "${:016X}: {}{}",
                start_address,
                bytes_stream.str(),
                chars_stream.str());
            size_t clip_length = value.length();
            if (clip_length > page_width)
                clip_length = static_cast<size_t>(page_width);

            auto column_offset = static_cast<size_t>(column());
            if (column_offset > 0
            &&  clip_length > column_offset) {
                clip_length -= column_offset;
            }
            value = value.substr(0, clip_length);

            mvwprintw(ptr(), row, 1, "%s", value.c_str());
        }
    }

};