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
#include "environment.h"
#include "header_window.h"

namespace basecode::debugger {

    header_window::header_window(
            window* parent,
            int x,
            int y,
            int width,
            int height) : window(parent,
                                 x,
                                 y,
                                 width,
                                 height,
                                 {}) {
    }

    void header_window::on_draw(environment& env) {
        auto& terp = env.session().terp();
        auto register_file = terp.register_file();

        std::stringstream flag_stream;
        uint64_t flag = 1;
        for (char s_flag : s_flags) {
            auto bit = register_file.flags(static_cast<vm::register_file_t::flags_t>(flag));
            flag_stream << fmt::format("{}:{} | ", s_flag, bit ? 1 : 0);
            flag <<= 1;
        }

        auto color_pair = 0;
        std::string mode;
        switch (env.current_state()) {
            case debugger_state_t::running: {
                mode = "running";
                break;
            }
            case debugger_state_t::stopped: {
                mode = "stopped";
                break;
            }
            case debugger_state_t::break_s: {
                mode = "break";
                break;
            }
            case debugger_state_t::single_step: {
                mode = "step";
                break;
            }
            case debugger_state_t::errored: {
                color_pair = 3;
                mode = "error";
                break;
            }
            case debugger_state_t::ended: {
                mode = "ended";
                break;
            }
            case debugger_state_t::command_entry: {
                color_pair = 4;
                mode = "command";
                break;
            }
            case debugger_state_t::command_execute: {
                mode = "execute";
                break;
            }
        }

        auto header = fmt::format(
            " Basecode Debugger | M: {} | HS: {} | SS: {} | {} ",
            mode,
            terp.heap_size(),
            terp.stack_size(),
            flag_stream.str());

        size_t pad_length = 0;
        if (header.length() < max_width())
            pad_length = max_width() - header.length();

        header = fmt::format(
            "{}{}",
            header,
            std::string(pad_length, ' '));

        leaveok(ptr(), true);
        mvwprintw(ptr(), 0, 0, "%s", header.c_str());
        mvwchgat(
            ptr(),
            0,
            0,
            max_width(),
            A_BOLD | A_REVERSE,
            color_pair,
            0);
    }

};