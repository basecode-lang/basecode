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

#include <vm/terp.h>
#include <vm/label.h>
#include <vm/assembler.h>
#include <common/defer.h>
#include <parser/token.h>
#include <compiler/session.h>
#include <common/string_support.h>
#include "environment.h"
#include "stack_window.h"
#include "header_window.h"
#include "footer_window.h"
#include "output_window.h"
#include "memory_window.h"
#include "errors_window.h"
#include "command_window.h"
#include "assembly_window.h"
#include "registers_window.h"

namespace basecode::debugger {

    std::unordered_map<command_type_t, command_handler_function_t> environment::s_command_handlers = {
        {command_type_t::help,          std::bind(&environment::on_help, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {command_type_t::find,          std::bind(&environment::on_find, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {command_type_t::goto_line,     std::bind(&environment::on_goto_line, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {command_type_t::show_memory,   std::bind(&environment::on_show_memory, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {command_type_t::read_memory,   std::bind(&environment::on_read_memory, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {command_type_t::write_memory,  std::bind(&environment::on_write_memory, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
    };

    ///////////////////////////////////////////////////////////////////////////

    environment::environment(compiler::session& session) :_session(session) {
    }

    environment::~environment() {
        delete _stack_window;
        delete _output_window;
        delete _header_window;
        delete _footer_window;
        delete _memory_window;
        delete _command_window;
        delete _assembly_window;
        delete _registers_window;
    }

    int environment::ch() const {
        return _ch;
    }

    void environment::draw_all() {
        _header_window->draw(*this);
        _footer_window->draw(*this);
        _assembly_window->draw(*this);
        _registers_window->draw(*this);
        _memory_window->draw(*this);
        _output_window->draw(*this);
        _stack_window->draw(*this);
        _command_window->draw(*this);
        _errors_window->draw(*this);
        refresh();
    }

    void environment::pop_state() {
        if (_state_stack.empty())
            return;
        _state_stack.pop();
    }

    void environment::cancel_command() {
        if (current_state() != debugger_state_t::command_entry)
            return;

        pop_state();
        _header_window->mark_dirty();
        _command_window->mark_dirty();
        draw_all();
    }

    bool environment::run(common::result& r) {
        auto& terp = _session.terp();

        _output_window->start_redirect();

        while (true) {
            auto pc = terp.register_file().r[vm::register_pc].qw;
            auto bp = breakpoint(pc);
            if (bp != nullptr
            &&  bp->enabled
            &&  current_state() != debugger_state_t::break_s) {
                push_state(debugger_state_t::break_s);
                _header_window->mark_dirty();
                _assembly_window->mark_dirty();
            }

            auto user_step = false;
            _ch = getch();
            switch (_ch) {
                case KEY_F(1): {
                    push_state(debugger_state_t::command_entry);
                    _header_window->mark_dirty();
                    _command_window->reset();
                    break;
                }
                case KEY_F(2): {
                    terp.reset();
                    pc = terp.register_file().r[vm::register_pc].qw;

                    unwind_state_stack();

                    _output_window->clear();
                    _stack_window->mark_dirty();
                    _header_window->mark_dirty();
                    _memory_window->mark_dirty();
                    _registers_window->mark_dirty();
                    _assembly_window->move_to_address(pc);
                    break;
                }
                case KEY_F(3): {
                    goto _exit;
                }
                case KEY_F(8): {
                    if (current_state() == debugger_state_t::break_s) {
                        pop_state();
                        if (current_state() == debugger_state_t::running) {
                            pop_state();
                            push_state(debugger_state_t::single_step);
                        }
                    }

                    if (current_state() == debugger_state_t::single_step) {
                        user_step = true;
                    } else {
                        // XXX: state error
                    }
                    break;
                }
                case KEY_F(9): {
                    switch (current_state()) {
                        case debugger_state_t::ended:
                        case debugger_state_t::errored:
                        case debugger_state_t::running:
                        case debugger_state_t::command_entry:
                        case debugger_state_t::command_execute: {
                            break;
                        }
                        case debugger_state_t::stopped: {
                            push_state(debugger_state_t::single_step);
                            _assembly_window->move_to_address(pc);
                            break;
                        }
                        case debugger_state_t::break_s: {
                            pop_state();
                            if (current_state() == debugger_state_t::single_step) {
                                pop_state();
                                push_state(debugger_state_t::running);
                            }
                            break;
                        }
                        case debugger_state_t::single_step: {
                            pop_state();
                            push_state(debugger_state_t::running);
                            break;
                        }
                    }
                    _header_window->mark_dirty();
                    break;
                }
                case CTRL('c'): {
                    if (current_state() == debugger_state_t::running)
                        pop_state();
                    _stack_window->mark_dirty();
                    _header_window->mark_dirty();
                    _output_window->mark_dirty();
                    _memory_window->mark_dirty();
                    _assembly_window->mark_dirty();
                    _registers_window->mark_dirty();
                    break;
                }
                case CTRL('r'): {
                    _registers_window->update(*this);
                    break;
                }
                case ERR: {
                    break;
                }
                default: {
                    switch (current_state()) {
                        case debugger_state_t::errored: {
                            _errors_window->update(*this);
                            _errors_window->mark_dirty();
                            break;
                        }
                        case debugger_state_t::command_entry:
                            _command_window->update(*this);
                            _command_window->mark_dirty();
                            _header_window->mark_dirty();
                            break;
                        default:
                            _assembly_window->update(*this);
                            break;
                    }
                    break;
                }
            }

            bool execute_next_step;
            if (user_step) {
                execute_next_step = current_state() == debugger_state_t::single_step;
            } else {
                execute_next_step = current_state() == debugger_state_t::running;
            };

            if (execute_next_step) {
                common::result step_result {};
                auto success = terp.step(step_result);
                if (!success) {
                    pop_state();
                    push_state(debugger_state_t::errored);
                    _errors_window->visible(true);
                } else {
                    pc = terp.register_file().r[vm::register_pc].qw;
                    if (terp.has_exited()) {
                        pop_state();
                        push_state(debugger_state_t::ended);
                    } else {
                        _assembly_window->move_to_address(pc);
                    }
                }
                _stack_window->mark_dirty();
                _header_window->mark_dirty();
                _memory_window->mark_dirty();
                _registers_window->mark_dirty();
            }

            _output_window->process_buffers();

            draw_all();
            refresh();
        }

    _exit:
        _output_window->stop_redirect();

        return true;
    }

    void environment::unwind_state_stack() {
        while (!_state_stack.empty())
            _state_stack.pop();
    }

    breakpoint_t* environment::add_breakpoint(
            uint64_t address,
            breakpoint_type_t type) {
        auto bp = breakpoint(address);
        if (bp != nullptr)
            return bp;
        auto it = _breakpoints.insert(std::make_pair(
            address,
            breakpoint_t{true, address, type}));
        return &it.first->second;
    }

    compiler::session& environment::session() {
        return _session;
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

        init_pair(1, COLOR_BLUE, COLOR_WHITE);
        init_pair(2, COLOR_BLUE, COLOR_YELLOW);
        init_pair(3, COLOR_RED, COLOR_WHITE);
        init_pair(4, COLOR_CYAN, COLOR_WHITE);

        _main_window = new window(nullptr, stdscr);
        _main_window->initialize();

        _header_window = new header_window(
            _main_window,
            0,
            0,
            _main_window->max_width(),
            1);
        _header_window->initialize();

        _footer_window = new footer_window(
            _main_window,
            0,
            _main_window->max_height() - 1,
            _main_window->max_width(),
            1);
        _footer_window->initialize();

        _command_window = new command_window(
            _main_window,
            0,
            _main_window->max_height() - 2,
            _main_window->max_width(),
            1);
        _command_window->initialize();

        _assembly_window = new assembly_window(
            _main_window,
            0,
            1,
            _main_window->max_width() - 23,
            _main_window->max_height() - 15);
        _assembly_window->initialize();

        _registers_window = new registers_window(
            _main_window,
            _main_window->max_width() - 23,
            1,
            23,
            _main_window->max_height() - 15);
        _registers_window->initialize();

        _stack_window = new stack_window(
            _main_window,
            _main_window->max_width() - 44,
            _main_window->max_height() - 14,
            44,
            12);
        _stack_window->initialize();

        auto left_section = _main_window->max_width() - _stack_window->width();
        _memory_window = new memory_window(
            _main_window,
            0,
            _main_window->max_height() - 14,
            left_section / 2,
            12);
        _memory_window->address(reinterpret_cast<uint64_t>(_session.terp().heap()));
        _memory_window->initialize();

        _output_window = new output_window(
            _main_window,
            _memory_window->x() + _memory_window->width(),
            _memory_window->y(),
            _memory_window->width(),
            _memory_window->height());
        _output_window->initialize();

        _errors_window = new errors_window(
            _main_window,
            2,
            2,
            _main_window->max_width() - 2,
            _main_window->max_height() - 2);
        _errors_window->visible(false);
        _errors_window->initialize();

        // XXX: since we only have the one listing source file for now
        //      automatically select it.
        auto& listing = _session.assembler().listing();
        auto file_names = listing.file_names();
        if (!file_names.empty())
            _assembly_window->source_file(listing.source_file(file_names.front()));

        refresh();
        draw_all();

        return true;
    }

    debugger_state_t environment::current_state() const {
        if (_state_stack.empty())
            return debugger_state_t::stopped;
        return _state_stack.top();
    }

    void environment::push_state(debugger_state_t state) {
        _state_stack.push(state);
    }

    void environment::remove_breakpoint(uint64_t address) {
        _breakpoints.erase(address);
    }

    breakpoint_t* environment::breakpoint(uint64_t address) {
        auto it = _breakpoints.find(address);
        if (it == _breakpoints.end())
            return nullptr;
        return &it->second;
    }

    bool environment::execute_command(const std::string& input) {
        if (input.empty()) {
            cancel_command();
            return true;
        }

        common::result result {};

        push_state(debugger_state_t::command_execute);
        _header_window->mark_dirty();
        _command_window->mark_dirty();
        draw_all();

        defer({
            pop_state();
            _command_window->reset();
            _header_window->mark_dirty();
            _command_window->mark_dirty();
            draw_all();
        });

        std::string part {};
        std::vector<std::string> parts {};

        size_t index = 0;
        auto in_quotes = false;
        while (index < input.size()) {
            auto c = input[index];
            if (isspace(c)) {
                if (!in_quotes) {
                    parts.emplace_back(part);
                    part.clear();
                } else {
                    part += " ";
                }
            } else if (isalnum(c)
                   ||  c == '_'
                   ||  c == '$'
                   ||  c == '.'
                   ||  c == ':'
                   ||  c == ';'
                   ||  c == '/'
                   ||  c == '%'
                   ||  c == '-'
                   ||  c == ','
                   ||  c == '@') {
                part += c;
            } else if (c == '\"' || c == '\'') {
                if (in_quotes) {
                    part += '\'';
                    parts.emplace_back(part);
                    part.clear();
                    in_quotes = false;
                } else {
                    part += '\'';
                    in_quotes = true;
                }
            }
            ++index;
        }
        if (!part.empty())
            parts.emplace_back(part);

        command_data_t cmd_data {};
        cmd_data.name = parts[0];
        if (!cmd_data.parse(result))
            return false;

        command_t cmd {};
        cmd.command = cmd_data;

        index = 1;
        for (const auto& kvp : cmd.command.prototype.params) {
            if (kvp.second.required && index < parts.size()) {
                auto param = parts[index];
                auto first_char = param[0];
                auto upper_first_char = std::toupper(param[0]);
                if (first_char == '@') {
                    number_data_t data {};
                    data.radix = 8;
                    data.input = param;
                    if (data.parse(data.value) != syntax::conversion_result_t::success)
                        break;
                    cmd.arguments.insert(std::make_pair(kvp.first, data));
                } else if (first_char == '%') {
                    number_data_t data {};
                    data.radix = 2;
                    data.input = param;
                    if (data.parse(data.value) != syntax::conversion_result_t::success)
                        return false;
                    cmd.arguments.insert(std::make_pair(kvp.first, data));
                } else if (first_char == '$') {
                    number_data_t data {};
                    data.radix = 16;
                    data.input = param;
                    if (data.parse(data.value) != syntax::conversion_result_t::success)
                        return false;
                    cmd.arguments.insert(std::make_pair(kvp.first, data));
                } else if (isdigit(first_char)) {
                    number_data_t data {};
                    data.radix = 10;
                    data.input = param;
                    if (data.parse(data.value) != syntax::conversion_result_t::success)
                        return false;
                    cmd.arguments.insert(std::make_pair(kvp.first, data));
                } else if (first_char == '\'') {
                    string_data_t data {};
                    data.input = param.substr(1, param.length() - 1);
                    cmd.arguments.insert(std::make_pair(kvp.first, data));
                } else if (first_char == '_') {
                    symbol_data_t data {};
                    data.input = param;
                } else if (upper_first_char == 'I'
                       || upper_first_char == 'F'
                       || upper_first_char == 'S'
                       || upper_first_char == 'P') {
                    register_data_t data {};
                    data.input = param;

                    common::result r;
                    if (!data.parse(r)) {
                        symbol_data_t symbol_data {};
                        symbol_data.input = param;
                        cmd.arguments.insert(std::make_pair(kvp.first, symbol_data));
                    } else {
                        cmd.arguments.insert(std::make_pair(kvp.first, data));
                    }
                } else {
                    result.error(
                        "X000",
                        fmt::format(
                            "unrecognized parameter '{}' value: {}",
                            kvp.second.name,
                            param));
                    break;
                }
            } else {
                result.error(
                    "X000",
                    fmt::format("missing required parameter: {}", kvp.second.name));
                break;
            }
            ++index;
        }

        if (result.is_failed()) {
            return false;
        }

        auto handler_it = s_command_handlers.find(cmd.command.prototype.type);
        if (handler_it == s_command_handlers.end()) {
            result.error(
                "X000",
                fmt::format("no command handler: {}", cmd.command.name));
            return false;
        }

        return handler_it->second(this, result, cmd);
    }

    uint64_t environment::get_address(const command_argument_t* arg) const {
        uint64_t address = 0;

        switch (arg->type()) {
            case command_parameter_type_t::symbol: {
                auto sym = arg->data<symbol_data_t>();
                if (sym != nullptr) {
                    auto label = _session.labels().find(sym->input);
                    if (label != nullptr)
                        address = label->address();
                }
                break;
            }
            case command_parameter_type_t::number: {
                auto number = arg->data<number_data_t>();
                if (number != nullptr)
                    address = number->value;
                break;
            }
            case command_parameter_type_t::register_t: {
                auto reg = arg->data<register_data_t>();
                if (reg != nullptr)
                    address = register_value(*reg);
                break;
            }
            default: {
                break;
            }
        }

        return address;
    }

    uint64_t environment::register_value(const register_data_t& reg) const {
        auto& terp = _session.terp();
        auto register_file = terp.register_file();

        uint64_t value = 0;
        switch (reg.value) {
            case vm::registers_t::pc: {
                value = register_file.r[vm::register_pc].qw + reg.offset;
                break;
            }
            case vm::registers_t::sp: {
                value = register_file.r[vm::register_sp].qw + reg.offset;
                break;
            }
            case vm::registers_t::fp: {
                value = register_file.r[vm::register_fp].qw + reg.offset;
                break;
            }
            case vm::registers_t::sr: {
                value = register_file.r[vm::register_sr].qw + reg.offset;
                break;
            }
            default: {
                if (reg.type == vm::register_type_t::integer) {
                    value = register_file.r[vm::register_integer_start + reg.value].qw + reg.offset;
                } else {
                    value = register_file.r[vm::register_float_start + reg.value].qw + reg.offset;
                }
                break;
            }
        }
        return value;
    }

    bool environment::on_help(common::result& r, const command_t& command) {
        return true;
    }

    bool environment::on_find(common::result& r, const command_t& command) {
        return true;
    }

    bool environment::on_goto_line(common::result& r, const command_t& command) {
        auto line_number_arg = command.arg("line_number");
        if (line_number_arg != nullptr) {
            auto line_number = line_number_arg->data<number_data_t>();
            _assembly_window->move_to_line(line_number->value);
        }
        return true;
    }

    bool environment::on_show_memory(common::result& r, const command_t& command) {
        auto address_arg = command.arg("address");
        if (address_arg != nullptr) {
            auto address = get_address(address_arg);
            if (address != 0) {
                _memory_window->address(address);
            }
        }
        return true;
    }

    bool environment::on_read_memory(common::result& r, const command_t& command) {
        auto address_arg = command.arg("address");
        if (address_arg != nullptr) {
            auto& terp = _session.terp();

            auto address = get_address(address_arg);
            if (address != 0) {
                auto value = terp.read(command.command.size, address);

                fmt::print("\n*read memory\n*-----------\n");
                fmt::print("*${:016X}: ", address);
                auto value_ptr = reinterpret_cast<uint8_t*>(&value);
                for (size_t i = 0; i < vm::op_size_in_bytes(command.command.size); i++)
                    fmt::print("{:02X} ", *value_ptr++);
                fmt::print("\n");
            }
        }
        return true;
    }

    bool environment::on_write_memory(common::result& r, const command_t& command) {
        return true;
    }

};