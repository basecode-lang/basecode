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
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////

    bool register_data_t::parse(common::result& r) {
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////

    command_prototype_t::size_flags_t command_prototype_t::suffix_to_size(const std::string& suffix) {
        if (suffix == ".b")
            return size_flags_t::byte;
        else if (suffix == ".w")
            return size_flags_t::word;
        else if (suffix == ".dw")
            return size_flags_t::dword;
        else if (suffix == ".qw")
            return size_flags_t::qword;
        return size_flags_t::none;
    }

    ///////////////////////////////////////////////////////////////////////////

    command_parameter_t::command_parameter_t() : _type(command_parameter_type_t::none) {
    }

    command_parameter_t::command_parameter_t(const symbol_data_t& value) : _data(value),
                                                                           _type(command_parameter_type_t::symbol) {
    }

    command_parameter_t::command_parameter_t(const string_data_t& value) : _data(value),
                                                                           _type(command_parameter_type_t::string_t) {
    }

    command_parameter_t::command_parameter_t(const number_data_t& value) : _data(value),
                                                                           _type(command_parameter_type_t::number) {
    }

    command_parameter_t::command_parameter_t(const register_data_t& value)  : _data(value),
                                                                              _type(command_parameter_type_t::register_t) {
    }

    command_parameter_type_t command_parameter_t::type() const {
        return _type;
    }

};