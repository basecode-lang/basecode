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

#include <fmt/format.h>
#include <common/string_support.h>
#include "debugger_types.h"

namespace basecode::debugger {

    syntax::conversion_result_t number_data_t::parse(double& value) const {
        syntax::token_t param;
        param.radix = radix;
        param.value = input;
        param.number_type = syntax::number_types_t::floating_point;
        return param.parse(value);
    }

    syntax::conversion_result_t number_data_t::parse(uint64_t& value) const {
        syntax::token_t param;
        param.radix = radix;
        param.value = input;
        param.number_type = syntax::number_types_t::integer;
        return param.parse(value);
    }

    ///////////////////////////////////////////////////////////////////////////

    bool command_data_t::parse(common::result& r) {
        auto parts = common::string_to_list(name, '.');
        if (parts.empty()) {
            r.add_message(
                "X000",
                "invalid command",
                true);
            return false;
        }

        name = parts[0];

        auto it = s_commands.find(name);
        if (it == s_commands.end()) {
            r.add_message(
                "X000",
                fmt::format("unknown command: {}", name),
                true);
            return false;
        }

        prototype = it->second;

        if (prototype.sizes != command_prototype_t::size_flags_t::none) {
            if (parts.size() < 2) {
                r.add_message(
                    "X000",
                    "command requires a size suffix",
                    true);
                return false;
            }
            auto size_flag = prototype.suffix_to_size(parts[1]);
            if ((prototype.sizes & size_flag) != 0) {
                switch (size_flag) {
                    case command_prototype_t::size_flags_t::none: {
                        r.add_message(
                            "X000",
                            "invalid size suffix for command",
                            true);
                        return false;
                    }
                    case command_prototype_t::size_flags_t::byte: {
                        size = vm::op_sizes::byte;
                        break;
                    }
                    case command_prototype_t::size_flags_t::word: {
                        size = vm::op_sizes::word;
                        break;
                    }
                    case command_prototype_t::size_flags_t::dword: {
                        size = vm::op_sizes::dword;
                        break;
                    }
                    case command_prototype_t::size_flags_t::qword: {
                        size = vm::op_sizes::qword;
                        break;
                    }
                }
            } else {
                r.add_message(
                    "X000",
                    "invalid size suffix for command",
                    true);
                return false;
            }
        } else {
            if (parts.size() == 2) {
                r.add_message(
                    "X000",
                    "command does not support size suffix",
                    true);
                return false;
            }
        }

        return true;
    }

    ///////////////////////////////////////////////////////////////////////////

    bool register_data_t::parse(common::result& r) {
        auto parts = common::string_to_list(input, ',');
        if (parts.empty()) {
            r.add_message(
                "X000",
                "invalid register",
                true);
            return false;
        }

        auto reg_name = parts[0];
        if (reg_name.length() < 2) {
            r.add_message(
                "X000",
                fmt::format("invalid register: {}", reg_name),
                true);
            return false;
        }

        std::string reg_offset;
        if (parts.size() == 2) {
            offset = std::atoi(parts[1].c_str());
        }

        auto reg_name_parts = common::string_to_list(reg_name, '.');
        reg_name = reg_name_parts[0];
        if (reg_name_parts.size() == 2) {
            auto suffix = reg_name_parts[1];
            if (suffix == "b")
                size = vm::op_sizes::byte;
            else if (suffix == "w")
                size = vm::op_sizes::word;
            else if (suffix == "dw")
                size = vm::op_sizes::dword;
            else if (suffix == "qw")
                size = vm::op_sizes::qword;
            else
                size = vm::op_sizes::none;
        }

        auto first_char = std::toupper(reg_name[0]);
        auto second_char = std::toupper(reg_name[1]);
        if (first_char == 'I') {
            auto number = std::atoi(reg_name.substr(1).c_str());
            value = static_cast<vm::registers_t>(number);
            type = vm::register_type_t::integer;
        } else if (first_char == 'F') {
            if (second_char == 'P') {
                value = vm::registers_t::fp;
                type = vm::register_type_t::integer;
            } else if (isdigit(second_char)) {
                auto number = std::atoi(reg_name.substr(1).c_str());
                value = static_cast<vm::registers_t>(number);
                type = vm::register_type_t::floating_point;
            } else {
                r.add_message(
                    "X000",
                    fmt::format("invalid register: {}", reg_name),
                    true);
                return false;
            }
        } else if (first_char == 'S') {
            if (second_char == 'P') {
                value = vm::registers_t::sp;
                type = vm::register_type_t::integer;
            } else if (second_char == 'R') {
                value = vm::registers_t::sr;
                type = vm::register_type_t::integer;
            } else {
                r.add_message(
                    "X000",
                    fmt::format("invalid register: {}", reg_name),
                    true);
                return false;
            }
        } else if (first_char == 'P' && second_char == 'C') {
            value = vm::registers_t::pc;
            type = vm::register_type_t::integer;
        }

        return true;
    }

    ///////////////////////////////////////////////////////////////////////////

    command_prototype_t::size_flags_t command_prototype_t::suffix_to_size(const std::string& suffix) {
        if (suffix == "b")
            return size_flags_t::byte;
        else if (suffix == "w")
            return size_flags_t::word;
        else if (suffix == "dw")
            return size_flags_t::dword;
        else if (suffix == "qw")
            return size_flags_t::qword;
        return size_flags_t::none;
    }

    ///////////////////////////////////////////////////////////////////////////

    command_argument_t::command_argument_t() : _type(command_parameter_type_t::none) {
    }

    command_argument_t::command_argument_t(const symbol_data_t& value) : _data(value),
                                                                         _type(command_parameter_type_t::symbol) {
    }

    command_argument_t::command_argument_t(const string_data_t& value) : _data(value),
                                                                         _type(command_parameter_type_t::string_t) {
    }

    command_argument_t::command_argument_t(const number_data_t& value) : _data(value),
                                                                         _type(command_parameter_type_t::number) {
    }

    command_argument_t::command_argument_t(const register_data_t& value)  : _data(value),
                                                                            _type(command_parameter_type_t::register_t) {
    }

    command_parameter_type_t command_argument_t::type() const {
        return _type;
    }

    const command_argument_t* command_t::arg(const std::string& name) const {
        auto it = arguments.find(name);
        if (it == arguments.end())
            return nullptr;
        return &it->second;
    }
};