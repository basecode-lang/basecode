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

#include <compiler/session.h>
#include "environment.h"

namespace basecode::debugger {

    environment::environment(compiler::session& session) : _session(session) {
    }

    void environment::update() {
        update_header();
        update_source();
        update_memory();
        update_stack();
        update_command();
        update_registers();
        update_footer();
        refresh();
    }

    void environment::update_stack() {
        auto& terp = _session.terp();
        auto sp = terp.register_file().r[vm::register_sp];
        auto top_of_stack = terp.heap_vector(vm::heap_vectors_t::top_of_stack);

        if (sp.qw == top_of_stack) {
            mvwprintw(_stack_window, 1, 1, "Stack empty");
        } else {
            int row = 1;
            while (sp.qw < top_of_stack) {
                auto value = fmt::format(
                    "${:016X}: ${:016X}",
                    sp.qw,
                    terp.read(vm::op_sizes::qword, sp.qw));
                mvwprintw(_stack_window, row, 1, value.c_str());
                sp.qw += 8;
                ++row;
            }
        }

        wrefresh(_stack_window);
    }

    void environment::update_memory() {
        auto& terp = _session.terp();
        auto address = reinterpret_cast<uint64_t>(terp.heap() + _memory_offset);

        for (int row = 1; row < 11; row++) {
            auto start_address = address;
            std::stringstream stream;
            for (size_t x = 0; x < 8; x++) {
                stream << fmt::format("{:02X} ", terp.read(vm::op_sizes::byte, address));
                address++;
            }
            auto value = fmt::format("${:016X}: {}", start_address, stream.str());
            mvwprintw(_memory_window, row, 1, value.c_str());
        }

        wrefresh(_memory_window);
    }

    void environment::update_footer() {
        auto footer = fmt::format(" F3 = Exit | {} ", "");
        footer = fmt::format(
            "{}{}",
            footer,
            std::string(_columns - footer.length(), ' '));
        leaveok(_footer_window, true);
        wattron(_footer_window, A_BOLD | A_REVERSE);
        mvwprintw(_footer_window, 0, 0, footer.c_str());
        wattroff(_footer_window, A_BOLD | A_REVERSE);
        wrefresh(_footer_window);
    }

    void environment::update_header() {
        auto& terp = _session.terp();
        auto register_file = terp.register_file();

        std::stringstream flag_stream;
        uint64_t flag = 1;
        for (char s_flag : s_flags) {
            auto bit = register_file.flags(static_cast<vm::register_file_t::flags_t>(flag));
            flag_stream << fmt::format("{}:{} | ", s_flag, bit ? 1 : 0);
            flag <<= 1;
        }

        auto header = fmt::format(
            " Basecode Debugger | Heap: {} | Stack: {} | {} ",
            terp.heap_size(),
            terp.stack_size(),
            flag_stream.str());
        size_t pad_length = 0;
        if (header.length() < _columns)
            pad_length = _columns - header.length();
        header = fmt::format(
            "{}{}",
            header,
            std::string(pad_length, ' '));
        leaveok(_header_window, true);
        wattron(_header_window, A_BOLD | A_REVERSE);
        mvwprintw(_header_window, 0, 0, header.c_str());
        wattroff(_header_window, A_BOLD | A_REVERSE);
        wrefresh(_header_window);
    }

    void environment::update_source() {
        auto& listing = _session.assembler().listing();

        wrefresh(_source_window);
    }

    void environment::update_command() {
        mvwprintw(_command_window, 0, 0, "> ");
        wrefresh(_command_window);
    }

    void environment::update_registers() {
        auto& terp = _session.terp();
        auto register_file = terp.register_file();

        auto pc_value = fmt::format(
            "PC=${:016X}",
            register_file.r[vm::register_pc].qw);
        mvwprintw(_registers_window, 1, 2, pc_value.c_str());

        auto sp_value = fmt::format(
            "SP=${:016X}",
            register_file.r[vm::register_sp].qw);
        mvwprintw(_registers_window, 2, 2, sp_value.c_str());

        auto fp_value = fmt::format(
            "FP=${:016X}",
            register_file.r[vm::register_fp].qw);
        mvwprintw(_registers_window, 3, 2, fp_value.c_str());

        int row = 5;
        for (uint8_t i = 0; i < 14; i++) {
            auto value = fmt::format(
                "I{:02}=${:016X}",
                i,
                register_file.r[i].qw);
            mvwprintw(_registers_window, row, 1, value.c_str());
            ++row;
        }

        ++row;
        for (uint8_t i = 0; i < 14; i++) {
            auto value = fmt::format(
                "F{:02}=${:016X}",
                i,
                register_file.r[vm::register_float_start + i].qw);
            mvwprintw(_registers_window, row, 1, value.c_str());
            ++row;
        }

        wrefresh(_registers_window);
    }

    bool environment::run(common::result& r) {
        while (true) {
            update();

            auto ch = getch();
            switch (ch) {
                case KEY_F(3): {
                    goto _exit;
                }
                case KEY_F(7): {
                    break;
                }
                case KEY_F(8): {
                    break;
                }
                case KEY_F(9): {
                    break;
                }
                default: {
                    break;
                }
            }
        }
    _exit:
        return true;
    }

    bool environment::shutdown(common::result& r) {
        endwin();
        return true;
    }

    bool environment::initialize(common::result& r) {
        initscr();
        scrollok(stdscr, false);

        start_color();

        raw();

        keypad(stdscr, true);

        noecho();

        getmaxyx(stdscr, _rows, _columns);

        init_pair(1, COLOR_BLUE, COLOR_WHITE);

        _header_window = make_window(1, _columns, 0, 0);
        scrollok(_header_window, false);

        _footer_window = make_window(1, _columns, _rows - 1, 0);
        scrollok(_footer_window, false);

        _source_window = make_window(_rows - 15, _columns - 23, 1, 0);
        scrollok(_source_window, true);
        title(_source_window, "Assembly Listing", _columns - 25);

        _registers_window = make_window(_rows - 15, 23, 1, _columns - 23);
        scrollok(_registers_window, true);
        title(_registers_window, "Registers", 21);

        _memory_window = make_window(12, _columns - 42, _rows - 14, 0);
        scrollok(_memory_window, true);
        title(_memory_window, "Memory (heap)", _columns - 44);

        _stack_window = make_window(12, 42, _rows - 14, _columns - 42);
        scrollok(_stack_window, true);
        title(_stack_window, "Stack", 40);

        _command_window = make_window(1, _columns, _rows - 2, 0);
        scrollok(_command_window, false);

        refresh();

        return true;
    }

    WINDOW* environment::make_window(int height, int width, int y, int x) {
        auto win = newwin(height, width, y, x);
        if (height > 1)
            box(win, 0, 0);
        return win;
    }

    void environment::title(WINDOW* win, const std::string& value, int width) {
        mvwhline(win, 0, 1, ' ', width);
        mvwprintw(win, 0, 2, value.c_str());
        mvwchgat(win, 0, 1, width, A_REVERSE, 1, 0);
    }

    void environment::print_centered_window(WINDOW* win, const std::string& value) {
        print_centered_row(win, _rows / 2, value);
    }

    void environment::print_centered_row(WINDOW* win, int row, const std::string& value) {
        mvwprintw(
            win,
            row,
            static_cast<int>((_columns - value.length()) / 2),
            "%s",
            value.c_str());
    }

};