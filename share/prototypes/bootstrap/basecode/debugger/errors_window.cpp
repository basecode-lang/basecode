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

#include "errors_window.h"

namespace basecode::debugger {

    errors_window::errors_window(
            window* parent,
            int x,
            int y,
            int width,
            int height) : window(parent,
                                 x,
                                 y,
                                 width,
                                 height,
                                 "Errors") {
    }

    void errors_window::move_up() {
    }

    void errors_window::page_up() {
    }

    void errors_window::page_down() {
    }

    void errors_window::move_down() {
    }

    void errors_window::move_left() {
    }

    void errors_window::move_right() {
    }

    void errors_window::on_draw(environment& env) {
    }

    common::result* errors_window::result() const {
        return _result;
    }

    bool errors_window::on_update(environment& env) {
        return false;
    }

    void errors_window::result(common::result* value) {
        _result = value;
    }

};