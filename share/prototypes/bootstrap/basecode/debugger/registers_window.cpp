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
#include "registers_window.h"

namespace basecode::debugger {

    registers_window::registers_window(
            window* parent,
            int x,
            int y,
            int width,
            int height) : window(parent,
                                 x,
                                 y,
                                 width,
                                 height,
                                 "Registers | CTRL+R") {
    }

    void registers_window::on_draw(environment& env) {
        auto& terp = env.session().terp();
        auto register_file = terp.register_file();

        auto pc_value = fmt::format(
            "PC=${:016X}",
            register_file.r[vm::register_pc].qw);
        mvwprintw(ptr(), 1, 2, "%s", pc_value.c_str());

        auto sp_value = fmt::format(
            "SP=${:016X}",
            register_file.r[vm::register_sp].qw);
        mvwprintw(ptr(), 2, 2, "%s", sp_value.c_str());

        auto fp_value = fmt::format(
            "FP=${:016X}",
            register_file.r[vm::register_fp].qw);
        mvwprintw(ptr(), 3, 2, "%s", fp_value.c_str());

        int row = 4;
        for (uint8_t i = 0; i < vm::number_general_purpose_registers; i++) {
            if (row > max_height() - 2)
                break;
            std::string value;
            switch (_mode) {
                case registers_display_mode_t::floats: {
                    value = fmt::format(
                        "{}F{}=${:016X}",
                        i < 10 ? " " : "",
                        i,
                        register_file.r[vm::register_float_start + i].qw);
                    break;
                }
                case registers_display_mode_t::integers: {
                    value = fmt::format(
                        "{}I{}=${:016X}",
                        i < 10 ? " " : "",
                        i,
                        register_file.r[i].qw);
                    break;
                }
            }
            mvwprintw(ptr(), row, 1, "%s", value.c_str());
            ++row;
        }
    }

    bool registers_window::on_update(environment& env) {
        switch (env.ch()) {
            case CTRL('r'): {
                if (_mode == registers_display_mode_t::integers)
                    _mode = registers_display_mode_t::floats;
                else
                    _mode = registers_display_mode_t::integers;

                mark_dirty();
                return true;
            }
            default:
                break;
        }

        return false;
    }

};