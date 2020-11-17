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

    class stack_window : public window {
    public:
        stack_window(
            window* parent,
            int x,
            int y,
            int width,
            int height);

    protected:
        void on_draw(environment& env) override;

        bool on_update(environment& env) override;

    private:
        int _current_line = 1;
    };

};

