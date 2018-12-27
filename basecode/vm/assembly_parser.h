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

#include <common/source_file.h>
#include "vm_types.h"

namespace basecode::vm {

    enum class wip_type_t {
        none = 0,
        mnemonic,
        directive
    };

    struct wip_t {
        void reset() {
            code.clear();
            params.clear();
            is_valid = false;
            operands.clear();
            instance.m = nullptr;
            size = op_sizes::none;
            type = wip_type_t::none;
        }

        std::string code {};
        bool is_valid = false;
        union {
            mnemonic_t* m = nullptr;
            directive_t* d;
        } instance;
        op_sizes size = op_sizes::none;
        wip_type_t type = wip_type_t::none;
        std::vector<operand_encoding_t> operands {};
        std::vector<directive_param_variant_t> params {};
    };

    enum class assembly_parser_state_t : uint8_t {
        start,
        whitespace,
        comment,
        label,
        mnemonic,
        operand_list,
        encode_instruction,
        encode_directive,
        directive,
        directive_param_list
    };

    class assembly_parser {
    public:
        assembly_parser(
            vm::assembler* assembler,
            common::source_file& source_file,
            const assembly_symbol_resolver_t& resolver);

        ~assembly_parser();

        bool parse(common::result& r);

    private:
        bool parse_immediate_number(
            common::result& r,
            const std::string& param,
            uint64_t& value);

        void parse_comma_separated_tokens(
            common::result& r,
            common::rune_t& rune,
            std::vector<std::string>& operand_strings);

        vm::assembly_symbol_type_t symbol_type(
            const std::string& operand,
            std::string& symbol);

        common::source_location make_location(size_t end_pos);

        bool is_float_register(const std::string& value) const;

        bool is_integer_register(const std::string& value) const;

    private:
        wip_t _wip {};
        size_t _start_pos = 0;
        assembly_parser_state_t _state;
        common::source_file& _source_file;
        vm::assembler* _assembler = nullptr;
        assembly_symbol_resolver_t _resolver = nullptr;
        std::unordered_map<std::string, vm::register_t> _locals {};
    };

};

