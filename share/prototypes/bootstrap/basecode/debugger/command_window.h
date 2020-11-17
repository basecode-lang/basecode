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

    class command_window : public window {
    public:
        command_window(
            window* parent,
            int x,
            int y,
            int width,
            int height);

        void reset();

    protected:
        void on_draw(environment& env) override;

        bool on_update(environment& env) override;

    private:
        void cursor_left();

        void cursor_right();

    private:
        size_t _cursor_x = 0;
        std::string _input {};
        size_t _history_index = 0;
        std::vector<std::string> _history {};
    };

};

