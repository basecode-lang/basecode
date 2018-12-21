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

#include <iostream>
#include <ncurses.h>
#include "debugger_types.h"

namespace basecode::debugger {

    enum class debugger_state_t : uint8_t {
        stopped,
        running,
        single_step,
    };

    enum class registers_display_mode_t : uint8_t {
        integers,
        floats
    };

    struct window_t {
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        int max_width = 0;
        int max_height = 0;
        short color_pair = 1;
        std::string title {};
        WINDOW* ptr = nullptr;
        bool scrollable = false;
    };

    class environment {
    public:
        explicit environment(compiler::session& session);

        bool run(common::result& r);

        bool shutdown(common::result& r);

        bool initialize(common::result& r);

    private:
        void draw_all();

        void draw_output(window_t& win);

        void draw_stack(window_t& win);

        void draw_header(window_t& win);

        void draw_footer(window_t& win);

        void draw_assembly(window_t& win);

        void draw_memory(window_t& win);

        void draw_command(window_t& win);

        void draw_registers(window_t& win);

        void print_centered_row(
            const window_t& win,
            int row,
            const std::string& value);

        void print_centered_window(
            const window_t& win,
            const std::string& value);

        size_t source_line_for_pc();

        void make_window(window_t& win);

        void title(const window_t& win);

    private:
        static inline char s_flags[] = {'Z', 'C', 'V', 'N', 'E', 'S'};

        window_t _main_window {};
        window_t _stack_window {};
        uint32_t _line_offset = 0;
        window_t _memory_window {};
        window_t _header_window {};
        window_t _footer_window {};
        uint32_t _current_line = 1;
        window_t _output_window {};
        uint32_t _output_offset = 0;
        window_t _command_window {};
        uint64_t _memory_offset = 0;
        compiler::session& _session;
        uint32_t _column_offset = 0;
        window_t _assembly_window {};
        window_t _registers_window {};
        std::vector<std::string> _stdout_lines {};
        vm::listing_source_file_t* _source_file = nullptr;
        debugger_state_t _state = debugger_state_t::stopped;
        registers_display_mode_t _registers_display_mode {};
    };

};
