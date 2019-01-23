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
#include "stack_window.h"

namespace basecode::debugger {

    stack_window::stack_window(
            window* parent,
            int x,
            int y,
            int width,
            int height) : window(parent,
                                 x,
                                 y,
                                 width,
                                 height,
                                 "Stack") {
    }

    void stack_window::on_draw(environment& env) {
        auto& terp = env.session().terp();
        auto sp = terp.register_file().r[vm::register_sp];
        auto top_of_stack = terp.heap_vector(vm::heap_vectors_t::top_of_stack);

        for (int row = 1; row < max_height() - 2; row++)
            mvwhline(ptr(), row, 1, ' ', max_width() - 2);

        if (sp.qw == top_of_stack) {
            print_centered_window("Stack is empty.");
        } else {
            int row = 1;

            while (sp.qw < top_of_stack && row < max_height() - 2) {
                auto value = fmt::format("${:016X}: ", sp.qw);
                for (auto x = 0; x < 8; x++) {
                    value += fmt::format("{:02X}", terp.read(vm::op_sizes::byte, sp.qw + x));
                    if (x < 7)
                        value += " ";
                }
                mvwprintw(ptr(), row, 1, "%s", value.c_str());
                sp.qw += 8;
                ++row;
            }

            mvwchgat(
                ptr(),
                _current_line,
                1,
                max_width() - 2,
                A_REVERSE,
                1,
                0);
        }
    }

    bool stack_window::on_update(environment& env) {
        return false;
    }

};