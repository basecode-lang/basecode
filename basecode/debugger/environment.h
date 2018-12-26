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

    class environment {
    public:
        explicit environment(compiler::session& session);

        ~environment();

        int ch() const;

        void cancel_command();

        bool run(common::result& r);

        compiler::session& session();

        breakpoint_t* add_breakpoint(
            uint64_t address,
            breakpoint_type_t type);

        bool shutdown(common::result& r);

        bool initialize(common::result& r);

        debugger_state_t current_state() const;

        void remove_breakpoint(uint64_t address);

        breakpoint_t* breakpoint(uint64_t address);

        bool execute_command(const std::string& input);

    private:
        void draw_all();

        void pop_state();

        void unwind_state_stack();

        void push_state(debugger_state_t state);

        uint64_t get_address(const command_argument_t* arg) const;

        uint64_t register_value(const register_data_t& reg) const;

    private:
        static std::unordered_map<command_type_t, command_handler_function_t> s_command_handlers;

        bool on_help(common::result& r, const command_t& command);

        bool on_find(common::result& r, const command_t& command);

        bool on_goto_line(common::result& r, const command_t& command);

        bool on_show_memory(common::result& r, const command_t& command);

        bool on_read_memory(common::result& r, const command_t& command);

        bool on_write_memory(common::result& r, const command_t& command);

    private:
        int _ch;
        compiler::session& _session;
        window* _main_window = nullptr;
        breakpoint_map_t _breakpoints {};
        stack_window* _stack_window = nullptr;
        errors_window* _errors_window = nullptr;
        output_window* _output_window = nullptr;
        header_window* _header_window = nullptr;
        footer_window* _footer_window = nullptr;
        memory_window* _memory_window = nullptr;
        command_window* _command_window = nullptr;
        assembly_window* _assembly_window = nullptr;
        std::stack<debugger_state_t> _state_stack {};
        registers_window* _registers_window = nullptr;
    };

};
