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

    void assembly_window::page_up() {
        for (auto i = 0; i < page_height(); i++)
            move_up();
    }

    void assembly_window::move_up() {
        if (_source_file == nullptr)
            return;

        if (_current_line > 1)
            _current_line--;
        else {
            auto current_row = row();
            if (current_row > page_height()) {
                _current_line = page_height();
                row(current_row - page_height());
            } else {
                row(0);
            }
        }
    }

    void assembly_window::move_top() {
        row(0);
        _current_line = 1;
    }

    void assembly_window::page_down() {
        for (auto i = 0; i < page_height(); i++)
            move_down();
    }

    void assembly_window::move_down() {
        if (_source_file == nullptr)
            return;

        auto max_lines = _source_file->lines.size();
        if (_current_line < page_height()) {
            if (_current_line + row() == max_lines)
                return;
            _current_line++;
        } else {
            auto current_row = row();
            if (current_row + page_height() < (max_lines - 1)) {
                row(current_row + page_height());
                _current_line = 1;
            }
        }
    }

    int assembly_window::page_width() const {
        return max_width() - 3;
    }

    int assembly_window::page_height() const {
        return max_height() - 2;
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

        auto line_number = row() + 1;
        for (size_t line_index = 0;
             line_index < page_height();
             line_index++) {
            mvwhline(ptr(), static_cast<int>(line_index + 1), 1, ' ', page_width() + 1);
            auto source_line_index = row() + line_index;
            if (source_line_index < _source_file->lines.size()) {
                const auto& line = _source_file->lines[source_line_index];
                auto value = fmt::format(
                    "{:06d}: ${:016X}  {}",
                    line_number++,
                    line.address,
                    line.source);

                size_t clip_length = value.length();
                if (clip_length > page_width())
                    clip_length = static_cast<size_t>(page_width());

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
            page_width() + 1,
            A_REVERSE,
            2,
            0);
    }

    bool assembly_window::on_update(environment& env) {
        switch (env.ch()) {
            case KEY_PPAGE: {
                page_up();
                mark_dirty();
                return true;
            }
            case KEY_NPAGE: {
                page_down();
                mark_dirty();
                return true;
            }
            case KEY_UP: {
                move_up();
                mark_dirty();
                return true;
            }
            case KEY_DOWN: {
                move_down();
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

        move_top();

        size_t index = 0;
        for (const auto& line : _source_file->lines) {
            if (line.address == address
            &&  line.type == vm::listing_source_line_type_t::instruction) {
                mark_dirty();
                return index;
            }
            move_down();
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