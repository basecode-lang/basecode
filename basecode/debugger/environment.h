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

#include <ncurses.h>
#include "debugger_types.h"

namespace basecode::debugger {

    class environment {
    public:
        explicit environment(compiler::session& session);

        bool run(common::result& r);

        bool shutdown(common::result& r);

        bool initialize(common::result& r);

    private:
        void update();

        void update_stack();

        void update_header();

        void update_footer();

        void update_source();

        void update_memory();

        void update_command();

        void update_registers();

        void title(WINDOW* win, const std::string& value, int width);

        WINDOW* make_window(int height, int width, int y, int x);

        void print_centered_window(WINDOW* win, const std::string& value);

        void print_centered_row(WINDOW* win, int row, const std::string& value);

    private:
        static inline char s_flags[] = {'Z', 'C', 'V', 'N', 'E', 'S'};

        int _rows = 0;
        int _columns = 0;
        uint64_t _memory_offset = 0;
        compiler::session& _session;
        WINDOW* _stack_window = nullptr;
        WINDOW* _memory_window = nullptr;
        WINDOW* _source_window = nullptr;
        WINDOW* _flags_window  = nullptr;
        WINDOW* _header_window = nullptr;
        WINDOW* _footer_window = nullptr;
        WINDOW* _command_window = nullptr;
        WINDOW* _registers_window = nullptr;
    };

};

