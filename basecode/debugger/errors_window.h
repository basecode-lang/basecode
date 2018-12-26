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

#pragma once

#include "window.h"

namespace basecode::debugger {

    class errors_window : public window {
    public:
        errors_window(
            window* parent,
            int x,
            int y,
            int width,
            int height);

        common::result* result() const;

        void result(common::result* value);

    protected:
        void on_draw(environment& env) override;

        bool on_update(environment& env) override;

    private:
        void move_up();

        void page_up();

        void page_down();

        void move_down();

        void move_left();

        void move_right();

    private:
        common::result* _result = nullptr;
    };

};

