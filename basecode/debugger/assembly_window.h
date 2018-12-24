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

    class assembly_window : public window {
    public:
        assembly_window(
            window* parent,
            int x,
            int y,
            int width,
            int height);

        void move_to_line(uint64_t line_number);

        size_t move_to_address(uint64_t address);

        void source_file(vm::listing_source_file_t* value);

    protected:
        void on_draw(environment& env) override;

        bool on_update(environment& env) override;

    private:
        void move_up();

        void page_up();

        void move_top();

        void move_down();

        void page_down();

        int page_width() const;

        int page_height() const;

    private:
        int _current_line = 1;
        vm::listing_source_file_t* _source_file = nullptr;
    };

};

