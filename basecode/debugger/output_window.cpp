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
#include "output_window.h"

namespace basecode::debugger {

    output_window::output_window(
            window* parent,
            int x,
            int y,
            int width,
            int height) : window(parent,
                                 x,
                                 y,
                                 width,
                                 height,
                                 "Output") {
    }

    void output_window::clear() {
        _stdout_lines.clear();
        _stderr_lines.clear();
        row(0);
        column(0);
        mark_dirty();
    }

    void output_window::stop_redirect() {
        std::fclose(_stdout_fp);
        stdout = _old_stdout_fp;
    }

    void output_window::start_redirect() {
        _stdout_fp = fmemopen(_stdout_buffer, 4096, "w");
        _old_stdout_fp = stdout;
        stdout = _stdout_fp;
    }

    void output_window::process_buffers() {
        std::string line;
        for (char val : _stdout_buffer) {
            if (val == 0)
                break;
            if (val == '\n') {
                mark_dirty();
                _stdout_lines.emplace_back(line);
                line.clear();
                if (_stdout_lines.size() > max_height() - 2)
                    row(row() + 1);
                continue;
            }
            line += val;
        }
        memset(_stdout_buffer, 0, 4096);
        std::fseek(_stdout_fp, 0, SEEK_SET);
    }

    void output_window::on_draw(environment& env) {
        auto page_width = max_width() - 2;
        auto page_height = max_height() - 2;
        for (size_t line_index = 0;
                line_index < page_height;
                line_index ++) {
            mvwhline(ptr(), static_cast<int>(line_index + 1), 1, ' ', page_width);
            auto output_line_index = row() + line_index;
            if (output_line_index < _stdout_lines.size()) {
                const auto& line = _stdout_lines[output_line_index];

                size_t clip_length = line.length();
                if (clip_length > page_width)
                    clip_length = static_cast<size_t>(page_width);

                auto column_offset = column();
                if (column_offset > 0
                &&  clip_length > column_offset) {
                    clip_length -= column_offset;
                }

                auto value = line.substr(0, clip_length);

                mvwprintw(
                    ptr(),
                    static_cast<int>(line_index + 1),
                    1,
                    "%s",
                    value.c_str());
            }
        }

        if (_stdout_lines.empty())
            print_centered_window("Output is empty.");
    }

};