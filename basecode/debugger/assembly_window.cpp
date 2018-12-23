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

#include <compiler/session.h>
#include <fmt/format.h>
#include "environment.h"
#include "assembly_window.h"

namespace basecode::debugger {

    assembly_window::assembly_window(
            window* parent,
            int x,
            int y,
            int width,
            int height) : window(parent,
                                 x,
                                 y,
                                 width,
                                 height,
                                 "Assembly") {
    }

    void assembly_window::on_draw(environment& env) {
        std::string file_name("(none)");
        auto total_lines = 0;
        if (_source_file != nullptr) {
            file_name = _source_file->path.filename().string();
            total_lines = static_cast<int>(_source_file->lines.size());
        }

        title(fmt::format(
            "Assembly | File: {} | Line: {} of {}",
            file_name,
            row() + _current_line,
            total_lines));

        auto page_size = max_height() - 2;
        auto page_width = max_width() - 3;

        auto line_number = row() + 1;
        for (size_t line_index = 0;
             line_index < page_size;
             line_index++) {
            mvwhline(ptr(), static_cast<int>(line_index + 1), 1, ' ', page_width + 1);
            auto source_line_index = row() + line_index;
            if (source_line_index < _source_file->lines.size()) {
                const auto& line = _source_file->lines[source_line_index];
                auto value = fmt::format(
                    "{:06d}: ${:016X}  {}",
                    line_number++,
                    line.address,
                    line.source);

                size_t clip_length = value.length();
                if (clip_length > page_width)
                    clip_length = static_cast<size_t>(page_width);

                auto column_offset = static_cast<size_t>(column());
                if (column_offset > 0
                &&  clip_length > column_offset) {
                    clip_length -= column_offset;
                }

                value = value.substr(column_offset, clip_length);

                mvwprintw(
                    ptr(),
                    static_cast<int>(line_index + 1),
                    2,
                    "%s",
                    value.c_str());
            }
        }

        mvwchgat(
            ptr(),
            _current_line,
            1,
            page_width + 1,
            A_REVERSE,
            2,
            0);
    }

    bool assembly_window::on_update(environment& env) {
        switch (env.ch()) {
            case KEY_PPAGE: {
                if (_source_file == nullptr)
                    break;

                auto page_size = max_height() - 2;
                if (row() > page_size)
                    row(row() - page_size);
                else
                    row(0);

                mark_dirty();
                return true;
            }
            case KEY_NPAGE: {
                if (_source_file == nullptr)
                    break;

                auto page_size = max_height() - 2;
                row(row() + page_size);
                if (row() > _source_file->lines.size() - 1)
                    row(static_cast<uint32_t>(_source_file->lines.size() - 1 - page_size));

                mark_dirty();
                return true;
            }
            case KEY_UP: {
                if (_source_file == nullptr)
                    break;

                if (_current_line > 1)
                    --_current_line;
                else {
                    if (row() > 0)
                        row(row() - 1);
                }

                mark_dirty();
                return true;
            }
            case KEY_DOWN: {
                if (_source_file == nullptr)
                    break;

                if (_current_line < max_height() - 2)
                    ++_current_line;
                else {
                    auto last_page_top = (_source_file->lines.size() - 1) - (max_height() - 2);
                    if (row() < last_page_top)
                        row(row() + 1);
                }

                mark_dirty();
                return true;
            }
            default:
                break;
        }

        return false;
    }

    size_t assembly_window::move_to_address(uint64_t address) {
        if (_source_file == nullptr)
            return 0;

        size_t index = 0;
        for (const auto& line : _source_file->lines) {
            if (line.address == address
            &&  line.type == vm::listing_source_line_type_t::instruction) {
                row(static_cast<int>(index));
                _current_line = 1;
                return index;
            }
            ++index;
        }
        return 0;
    }

    void assembly_window::source_file(vm::listing_source_file_t* value) {
        if (value != _source_file) {
            _source_file = value;
            mark_dirty();
        }
    }

};