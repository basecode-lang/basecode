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
#include "command_window.h"

namespace basecode::debugger {

    command_window::command_window(
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

    void command_window::on_draw(environment& env) {
        mvwprintw(ptr(), 0, 0, "%s", "(command) > ");
    }

};