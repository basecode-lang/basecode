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
#include <common/string_support.h>
#include "environment.h"

namespace basecode::debugger {

    environment::environment(compiler::session& session) : _session(session) {
    }

    void environment::draw_all() {
        draw_header(_header_window);
        draw_assembly(_assembly_window);
        draw_memory(_memory_window);
        draw_stack(_stack_window);
        draw_command(_command_window);
        draw_output(_output_window);
        draw_registers(_registers_window);
        draw_footer(_footer_window);
        refresh();
    }

    void environment::draw_stack(window_t& win) {
        auto& terp = _session.terp();
        auto sp = terp.register_file().r[vm::register_sp];
        auto top_of_stack = terp.heap_vector(vm::heap_vectors_t::top_of_stack);

        for (int row = 1; row < win.max_height - 2; row++)
            mvwhline(win.ptr, row, 1, ' ', win.max_width - 2);

        if (sp.qw == top_of_stack) {
            print_centered_window(win, "Stack is empty.");
        } else {
            int row = 1;
            while (sp.qw < top_of_stack && row < win.max_height - 2) {
                auto value = fmt::format(
                    "${:016X}: ${:016X}",
                    sp.qw,
                    terp.read(vm::op_sizes::qword, sp.qw));
                mvwprintw(win.ptr, row, 1, "%s", value.c_str());
                sp.qw += 8;
                ++row;
            }
        }

        wrefresh(win.ptr);
    }

    void environment::draw_memory(window_t& win) {
        auto& terp = _session.terp();
        auto address = reinterpret_cast<uint64_t>(terp.heap() + _memory_offset);

        for (int row = 1; row < 11; row++) {
            auto start_address = address;
            std::stringstream bytes_stream;
            std::stringstream chars_stream;
            for (size_t x = 0; x < 8; x++) {
                auto value = terp.read(vm::op_sizes::byte, address);
                bytes_stream << fmt::format("{:02X} ", value);
                chars_stream << (char)(isprint(static_cast<int>(value)) ? value : '.');
                address++;
            }
            auto value = fmt::format(
                "${:016X}: {}{}",
                start_address,
                bytes_stream.str(),
                chars_stream.str());
            mvwprintw(win.ptr, row, 1, "%s", value.c_str());
        }

        wrefresh(win.ptr);
    }

    void environment::draw_footer(window_t& win) {
        auto footer = fmt::format(" F1=Command | F2=Reset | F3=Exit | F8=Step | F9=Run {} ", "");

        size_t pad_length = 0;
        if (footer.length() < _main_window.max_width)
            pad_length = _main_window.max_width - footer.length();

        footer = fmt::format(
            "{}{}",
            footer,
            std::string(pad_length, ' '));

        leaveok(win.ptr, true);
        wattron(win.ptr, A_BOLD | A_REVERSE);
        mvwprintw(win.ptr, 0, 0, "%s", footer.c_str());
        wattroff(win.ptr, A_BOLD | A_REVERSE);
        wrefresh(win.ptr);
    }

    void environment::draw_header(window_t& win) {
        auto& terp = _session.terp();
        auto register_file = terp.register_file();

        std::stringstream flag_stream;
        uint64_t flag = 1;
        for (char s_flag : s_flags) {
            auto bit = register_file.flags(static_cast<vm::register_file_t::flags_t>(flag));
            flag_stream << fmt::format("{}:{} | ", s_flag, bit ? 1 : 0);
            flag <<= 1;
        }

        std::string mode;
        switch (_state) {
            case debugger_state_t::running: {
                mode = "running";
                break;
            }
            case debugger_state_t::stopped: {
                mode = "stopped";
                break;
            }
            case debugger_state_t::single_step: {
                mode = "step";
                break;
            }
        }

        auto header = fmt::format(
            " Basecode Debugger | M: {} | HS: {} | SS: {} | {} ",
            mode,
            terp.heap_size(),
            terp.stack_size(),
            flag_stream.str());

        size_t pad_length = 0;
        if (header.length() < _main_window.max_width)
            pad_length = _main_window.max_width - header.length();

        header = fmt::format(
            "{}{}",
            header,
            std::string(pad_length, ' '));

        leaveok(win.ptr, true);
        wattron(win.ptr, A_BOLD | A_REVERSE);
        mvwprintw(win.ptr, 0, 0, "%s", header.c_str());
        wattroff(win.ptr, A_BOLD | A_REVERSE);
        wrefresh(win.ptr);
    }

    void environment::draw_assembly(window_t& win) {
        std::string file_name("(none)");
        auto total_lines = 0;
        if (_source_file != nullptr) {
            file_name = _source_file->path.filename().string();
            total_lines = static_cast<int>(_source_file->lines.size());
        }
        win.title = fmt::format(
            "Assembly | File: {} | Line: {} of {}",
            file_name,
            _line_offset + _current_line,
            total_lines);
        title(win);

        auto page_size = win.max_height - 2;
        auto page_width = win.max_width - 3;

        auto line_number = _line_offset + 1;
        for (size_t line_index = 0;
                 line_index < page_size;
                 line_index++) {
            mvwhline(win.ptr, static_cast<int>(line_index + 1), 1, ' ', page_width);
            auto source_line_index = _line_offset + line_index;
            if (source_line_index < _source_file->lines.size()) {
                const auto& line = _source_file->lines[source_line_index];
                auto value = fmt::format(
                    "{:06d}: ${:016X}  {}",
                    line_number++,
                    line.address,
                    line.source);
                size_t clip_length = value.length();
                if (clip_length > page_width)
                    clip_length = static_cast<size_t>(page_width);
                if (_column_offset > 0
                    && clip_length > _column_offset) {
                    clip_length -= _column_offset;
                }
                value = value.substr(_column_offset, clip_length);
                mvwprintw(
                    win.ptr,
                    static_cast<int>(line_index + 1),
                    2,
                    "%s",
                    value.c_str());
            }
        }

        mvwchgat(
            win.ptr,
            _current_line,
            1,
            page_width,
            A_REVERSE,
            2,
            0);

        wrefresh(win.ptr);
    }

    void environment::draw_output(window_t& win) {
        auto page_width = win.max_width - 2;
        auto page_height = win.max_height - 2;
        for (size_t line_index = 0;
                line_index < page_height;
                line_index ++) {
            mvwhline(win.ptr, static_cast<int>(line_index + 1), 1, ' ', page_width);
            auto output_line_index = _output_offset + line_index;
            if (output_line_index < _stdout_lines.size()) {
                const auto& line = _stdout_lines[output_line_index];
                mvwprintw(
                    win.ptr,
                    static_cast<int>(line_index + 1),
                    1,
                    "%s",
                    line.c_str());
            }
        }
        if (_stdout_lines.empty())
            print_centered_window(win, "Output is empty.");
        wrefresh(win.ptr);
    }

    void environment::draw_command(window_t& win) {
        mvwprintw(win.ptr, 0, 0, "%s", "(command) > ");
        wrefresh(win.ptr);
    }

    void environment::draw_registers(window_t& win) {
        auto& terp = _session.terp();
        auto register_file = terp.register_file();

        auto pc_value = fmt::format(
            "PC=${:016X}",
            register_file.r[vm::register_pc].qw);
        mvwprintw(win.ptr, 1, 2, "%s", pc_value.c_str());

        auto sp_value = fmt::format(
            "SP=${:016X}",
            register_file.r[vm::register_sp].qw);
        mvwprintw(win.ptr, 2, 2, "%s", sp_value.c_str());

        auto fp_value = fmt::format(
            "FP=${:016X}",
            register_file.r[vm::register_fp].qw);
        mvwprintw(win.ptr, 3, 2, "%s", fp_value.c_str());

        int row = 4;
        for (uint8_t i = 0; i < 32; i++) {
            if (row > win.max_height - 2)
                break;
            std::string value;
            switch (_registers_display_mode) {
                case registers_display_mode_t::floats: {
                    value = fmt::format(
                        "F{:02}=${:016X}",
                        i,
                        register_file.r[vm::register_float_start + i].qw);
                    break;
                }
                case registers_display_mode_t::integers: {
                    value = fmt::format(
                        "I{:02}=${:016X}",
                        i,
                        register_file.r[i].qw);
                    break;
                }
            }
            mvwprintw(win.ptr, row, 1, "%s", value.c_str());
            ++row;
        }

        wrefresh(win.ptr);
    }

    void environment::print_centered_row(
            const window_t& win,
            int row,
            const std::string& value) {
        mvwprintw(
            win.ptr,
            row,
            static_cast<int>((win.max_width - value.length()) / 2),
            "%s",
            value.c_str());
    }

    bool environment::run(common::result& r) {
        auto& terp = _session.terp();

        char buffer[4096];
        auto fp = fmemopen(buffer, 4096, "w");
        auto old_stdout = stdout;
        stdout = fp;

        while (true) {
            auto user_step = false;

            auto ch = getch();
            switch (ch) {
                case KEY_F(2): {
                    terp.reset();
                    _state = debugger_state_t::stopped;
                    _stdout_lines.clear();
                    _current_line = 1;
                    _line_offset = static_cast<uint32_t>(source_line_for_pc());
                    draw_all();
                    break;
                }
                case KEY_F(3): {
                    goto _exit;
                }
                case KEY_F(8): {
                    if (_state != debugger_state_t::single_step) {
                        // XXX: error!
                        break;
                    }
                    user_step = true;
                    break;
                }
                case KEY_F(9): {
                    switch (_state) {
                        case debugger_state_t::running: {
                            break;
                        }
                        case debugger_state_t::stopped: {
                            _current_line = 1;
                            _line_offset = static_cast<uint32_t>(source_line_for_pc());
                            _state = debugger_state_t::single_step;
                            break;
                        }
                        case debugger_state_t::single_step: {
                            _state = debugger_state_t::running;
                            break;
                        }
                    }
                    draw_header(_header_window);
                    draw_assembly(_assembly_window);
                    draw_registers(_registers_window);
                    break;
                }
                case KEY_PPAGE: {
                    if (_source_file == nullptr)
                        break;

                    auto page_size = _assembly_window.max_height - 2;
                    if (_line_offset > page_size)
                        _line_offset -= page_size;
                    else
                        _line_offset = 0;

                    draw_assembly(_assembly_window);
                    break;
                }
                case KEY_NPAGE: {
                    if (_source_file == nullptr)
                        break;

                    auto page_size = _assembly_window.max_height - 2;
                    _line_offset += page_size;
                    if (_line_offset > _source_file->lines.size() - 1)
                        _line_offset = static_cast<uint32_t>(_source_file->lines.size() - 1 - page_size);

                    draw_assembly(_assembly_window);
                    break;
                }
                case KEY_UP: {
                    if (_source_file == nullptr)
                        break;

                    if (_current_line > 1)
                        --_current_line;
                    else {
                        if (_line_offset > 0)
                            --_line_offset;
                    }

                    draw_assembly(_assembly_window);
                    break;
                }
                case KEY_DOWN: {
                    if (_source_file == nullptr)
                        break;

                    if (_current_line < _assembly_window.max_height - 2)
                        ++_current_line;
                    else {
                        auto last_page_top = (_source_file->lines.size() - 1) - _assembly_window.max_height - 2;
                        if (_line_offset < last_page_top)
                            ++_line_offset;
                    }

                    draw_assembly(_assembly_window);
                    break;
                }
                case CTRL('c'): {
                    if (_state == debugger_state_t::running)
                        _state = debugger_state_t::stopped;
                    break;
                }
                case CTRL('r'): {
                    if (_registers_display_mode == registers_display_mode_t::integers)
                        _registers_display_mode = registers_display_mode_t::floats;
                    else
                        _registers_display_mode = registers_display_mode_t::integers;

                    draw_registers(_registers_window);
                    break;
                }
                case ERR: {
                    break;
                }
                default: {
                    break;
                }
            }

            refresh();

            bool execute_next_step;
            if (user_step) {
                execute_next_step = _state == debugger_state_t::single_step;
            } else {
                execute_next_step = _state == debugger_state_t::running;
            };

            if (execute_next_step) {
                auto success = terp.step(_session.result());
                if (!success) {
                    // XXX:
                    continue;
                }

                std::string line;
                for (size_t i = 0; i < 4096; i++) {
                    auto val = buffer[i];
                    if (val == 0)
                        break;
                    if (val == '\n') {
                        _stdout_lines.emplace_back(line);
                        line.clear();
                        if (_stdout_lines.size() > _output_window.max_height - 2)
                            _output_offset++;
                        continue;
                    }
                    line += val;
                }
                memset(buffer, 0, 4096);
                std::fseek(fp, 0, SEEK_SET);

                if (terp.has_exited()) {
                    _state = debugger_state_t::stopped;
                } else {
                    _current_line = 1;
                    _line_offset = static_cast<uint32_t>(source_line_for_pc());
                }
                draw_all();
            }
        }

    _exit:
        std::fclose(fp);
        stdout = old_stdout;

        return true;
    }

    bool environment::shutdown(common::result& r) {
        endwin();
        return true;
    }

    bool environment::initialize(common::result& r) {
        initscr();

        start_color();

        raw();

        keypad(stdscr, true);

        noecho();
        nodelay(stdscr, true);

        _main_window.ptr = stdscr;
        make_window(_main_window);

        init_pair(1, COLOR_BLUE, COLOR_WHITE);
        init_pair(2, COLOR_BLUE, COLOR_YELLOW);

        _header_window.x = 0;
        _header_window.y = 0;
        _header_window.height = 1;
        _header_window.width = _main_window.max_width;
        make_window(_header_window);

        _footer_window.x = 0;
        _footer_window.height = 1;
        _footer_window.y = _main_window.max_height - 1;
        _footer_window.width = _main_window.max_width;
        make_window(_footer_window);

        _assembly_window.x = 0;
        _assembly_window.y = 1;
        _assembly_window.title = "Assembly";
        _assembly_window.width = _main_window.max_width - 23;
        _assembly_window.height = _main_window.max_height - 15;
        make_window(_assembly_window);

        _registers_window.y = 1;
        _registers_window.width = 23;
        _registers_window.title = "Registers | CTRL+R";
        _registers_window.x = _main_window.max_width - 23;
        _registers_window.height = _main_window.max_height - 15;
        make_window(_registers_window);

        auto left_section = _main_window.max_width - 38;
        _memory_window.x = 0;
        _memory_window.width = left_section / 2;
        _memory_window.height = 12;
        _memory_window.title = "Memory (heap)";
        _memory_window.y = _main_window.max_height - 14;
        make_window(_memory_window);

        _output_window.scrollable = true;
        _output_window.x = _memory_window.width;
        _output_window.width = left_section / 2;
        _output_window.height = 12;
        _output_window.title = "Output";
        _output_window.y = _main_window.max_height - 14;
        make_window(_output_window);

        _stack_window.width = 38;
        _stack_window.height = 12;
        _stack_window.title = "Stack";
        _stack_window.y = _main_window.max_height - 14;
        _stack_window.x = _main_window.max_width - _stack_window.width;
        make_window(_stack_window);

        _command_window.x = 0;
        _command_window.height = 1;
        _command_window.width = _main_window.max_width;
        _command_window.y = _main_window.max_height - 2;
        make_window(_command_window);

        // XXX: since we only have the one listing source file for now
        //      automatically select it.
        auto& listing = _session.assembler().listing();
        auto file_names = listing.file_names();
        if (!file_names.empty())
            _source_file = listing.source_file(file_names.front());

        refresh();
        draw_all();

        return true;
    }

    void environment::make_window(window_t& win) {
        if (win.ptr == nullptr)
            win.ptr = newwin(win.height, win.width, win.y, win.x);

        if (win.height > 1)
            box(win.ptr, 0, 0);

        scrollok(win.ptr, win.scrollable);

        getmaxyx(win.ptr, win.max_height, win.max_width);

        if (!win.title.empty())
            title(win);

        wrefresh(win.ptr);
    }

    void environment::title(const window_t& win) {
        mvwhline(win.ptr, 0, 1, ' ', win.max_width - 2);
        mvwprintw(win.ptr, 0, 2, "%s", win.title.c_str());
        mvwchgat(win.ptr, 0, 1, win.max_width - 2, A_REVERSE, win.color_pair, 0);
    }

    void environment::print_centered_window(
            const window_t& win,
            const std::string& value) {
        print_centered_row(win, win.max_height / 2, value);
    }

    size_t environment::source_line_for_pc() {
        if (_source_file == nullptr)
            return 0;

        auto& terp = _session.terp();
        auto pc = terp.register_file().r[vm::register_pc].qw;

        size_t index = 0;
        for (const auto& line : _source_file->lines) {
            if (line.address == pc
            &&  line.type == vm::listing_source_line_type_t::instruction) {
                return index;
            }
            ++index;
        }

        return 0;
    }

};