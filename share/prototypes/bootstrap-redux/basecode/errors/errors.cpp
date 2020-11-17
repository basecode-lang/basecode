// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
//       C O M P I L E R  P R O J E C T
//
// Copyright (C) 2019 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <basecode/adt/hash_table.h>
#include <basecode/strings/transforms.h>
#include <basecode/memory/frame_allocator.h>
#include "errors.h"

namespace basecode::errors {

    struct error_system_t final {
        error_system_t(
            memory::allocator_t* allocator,
            error_decl_init_list_t elements) : locale(allocator),
                                               allocator(allocator),
                                               decls(elements, allocator) {
        }

        adt::string_t locale;
        strings::pool_t* pool{};
        memory::allocator_t* allocator;
        memory::frame_allocator_t<4096>* pool_allocator{};
        adt::hash_table_t<error_decl_key_t, error_decl_t> decls;
    };

    error_system_t* g_error_system{};

    bool shutdown() {
        auto allocator = g_error_system->allocator;

        memory::destroy(allocator, g_error_system->pool);
        memory::destroy(allocator, g_error_system->pool_allocator);
        memory::destroy(allocator, g_error_system);

        return true;
    }

    bool initialize(
            result_t& r,
            memory::allocator_t* allocator) {
        const error_decl_init_list_t elements = {
            // ----------------------
            // io
            // ----------------------

            {
                {.code = io::unable_to_read_file, .locale = "en_US"sv},
                {.code = "I001"sv, .message = "unable to read file: {}"sv}
            },

            {
                {.code = io::unable_to_write_file, .locale = "en_US"sv},
                {.code = "I002"sv, .message = "unable to write file: {}"sv}
            },

            // ----------------------
            // lexer
            // ----------------------

            {
                {.code = lexer::unable_to_convert_integer_value, .locale = "en_US"sv},
                {.code = "L001"sv, .message = "unable to convert integer value {} because {}"sv}
            },

            {
                {.code = lexer::unable_to_narrow_integer_value, .locale = "en_US"sv},
                {.code = "L002"sv, .message = "unable to narrow integer value"sv}
            },

            {
                {.code = lexer::unable_to_convert_floating_point_value, .locale = "en_US"sv},
                {.code = "L003"sv, .message = "unable to convert floating point value {} because {}"sv}
            },

            {
                {.code = lexer::unable_to_narrow_floating_point_value, .locale = "en_US"sv},
                {.code = "L004"sv, .message = "unable to narrow floating point value"sv}
            },

            {
                {.code = lexer::invalid_identifier_start_character, .locale = "en_US"sv},
                {.code = "L005"sv, .message = "identifiers must start with _ or a letter; found: {}"sv}
            },

            {
                {.code = lexer::expected_identifier, .locale = "en_US"sv},
                {.code = "L006"sv, .message = "expected identifier"sv}
            },

            {
                {.code = lexer::unexpected_end_of_input, .locale = "en_US"sv},
                {.code = "L007"sv, .message = "unexpected end of input"sv}
            },

            {
                {.code = lexer::unexpected_decimal_point, .locale = "en_US"sv},
                {.code = "L008"sv, .message = "unexpected decimal point"sv}
            },

            {
                {.code = lexer::expected_closing_single_quote, .locale = "en_US"sv},
                {.code = "L009"sv, .message = "expected closing ' but found: {}"sv}
            },

            {
                {.code = lexer::expected_directive_prefix, .locale = "en_US"sv},
                {.code = "L010"sv, .message = "expected directive prefix: #"sv}
            },

            {
                {.code = lexer::expected_annotation_prefix, .locale = "en_US"sv},
                {.code = "L011"sv, .message = "expected annotation prefix: @"sv}
            },

            {
                {.code = lexer::exponent_notation_not_valid_for_integers, .locale = "en_US"sv},
                {.code = "L012"sv, .message = "exponent notation is not valid for integer literals"sv}
            },

            {
                {.code = lexer::unexpected_letter_after_decimal_number_literal, .locale = "en_US"sv},
                {.code = "L013"sv, .message = "unexpected letter immediately after decimal number"sv}
            },

            {
                {.code = lexer::expected_hex_literal_prefix, .locale = "en_US"sv},
                {.code = "L014"sv, .message = "expected hex prefix: $"sv}
            },

            {
                {.code = lexer::unexpected_letter_after_hexadecimal_number_literal, .locale = "en_US"sv},
                {.code = "L015"sv, .message = "unexpected letter immediately after hexadecimal number"sv}
            },

            {
                {.code = lexer::expected_octal_literal_prefix, .locale = "en_US"sv},
                {.code = "L016"sv, .message = "expected octal prefix: @"sv}
            },

            {
                {.code = lexer::unexpected_letter_after_octal_number_literal, .locale = "en_US"sv},
                {.code = "L017"sv, .message = "unexpected letter immediately after octal number"sv}
            },

            {
                {.code = lexer::expected_binary_literal_prefix, .locale = "en_US"sv},
                {.code = "L018"sv, .message = "expected binary prefix: %"sv}
            },

            {
                {.code = lexer::unexpected_letter_after_binary_number_literal, .locale = "en_US"sv},
                {.code = "L019"sv, .message = "unexpected letter or non-binary digit immediately after binary number"sv}
            },

            {
                {.code = lexer::expected_closing_block_literal, .locale = "en_US"sv},
                {.code = "L020"sv, .message = "expected }} but found: {}"sv}
            },

            {
                {.code = lexer::unescaped_quote, .locale = "en_US"sv},
                {.code = "L021"sv, .message = "an unescaped quote was detected"sv}
            },

            // ----------------------
            // parser
            // ----------------------

            {
                {.code = parser::invalid_token, .locale = "en_US"sv},
                {.code = "P001"sv, .message = "a token has not been properly configured for parsing"sv}
            },

            {
                {.code = parser::undefined_production_rule, .locale = "en_US"sv},
                {.code = "P002"sv, .message = "undefined production rule"sv}
            },

            {
                {.code = parser::missing_operator_production_rule, .locale = "en_US"sv},
                {.code = "P003"sv, .message = "missing operator production rule"sv}
            },

            {
                {.code = parser::unexpected_token, .locale = "en_US"sv},
                {.code = "P004"sv, .message = "expected token {} but encountered {}"sv}
            },

            {
                {.code = parser::member_select_operator_requires_identifier_lvalue, .locale = "en_US"sv},
                {.code = "P005"sv, .message = "member select requires identifier lvalue"sv}
            },

            {
                {.code = parser::member_select_operator_requires_identifier_rvalue, .locale = "en_US"sv},
                {.code = "P006"sv, .message = "member select requires identifier rvalue"sv}
            },

            {
                {.code = parser::expected_expression, .locale = "en_US"sv},
                {.code = "P007"sv, .message = "expected expression"sv}
            },

            {
                {.code = parser::invalid_assignment_expression, .locale = "en_US"sv},
                {.code = "P008"sv, .message = "invalid assignment expression"sv}
            },

            {
                {.code = parser::assignment_requires_valid_lvalue, .locale = "en_US"sv},
                {.code = "P009"sv, .message = "assignment requires valid lvalue"sv}
            },

            // ----------------------
            // graphviz
            // ----------------------

            {
                {.code = graphviz::attribute_type_not_found, .locale = "en_US"sv},
                {.code = "G001"sv, .message = "attribute type not found"sv}
            },

            {
                {.code = graphviz::invalid_attribute_for_component, .locale = "en_US"sv},
                {.code = "G002"sv, .message = "invalid attriubte '{}' for component '{}'"sv}
            },

            // ----------------------
            // source_buffer_t
            // ----------------------

            {
                {.code = source_buffer::unable_to_open_file, .locale = "en_US"sv},
                {.code = "S001"sv, .message = "unable to open source file: {}"sv}
            },

            // ----------------------
            // utf8_module
            // ----------------------

            {
                {.code = utf8_module::unable_to_open_file, .locale = "en_US"sv},
                {.code = "S001"sv, .message = "unable to open source file: {}"sv}
            },

            {
                {.code = utf8_module::at_end_of_buffer, .locale = "en_US"sv},
                {.code = "S002"sv, .message = "at end of buffer"sv}
            },

            {
                {.code = utf8_module::at_beginning_of_buffer, .locale = "en_US"sv},
                {.code = "S003"sv, .message = "at beginning of buffer"sv}
            },

            {
                {.code = utf8_module::illegal_encoding, .locale = "en_US"sv},
                {.code = "S004"sv, .message = "illegal utf-8 encoding"sv}
            },

            {
                {.code = utf8_module::illegal_nul_character, .locale = "en_US"sv},
                {.code = "S005"sv, .message = "illegal nul character"sv}
            },

            {
                {.code = utf8_module::illegal_byte_order_mark, .locale = "en_US"sv},
                {.code = "S006"sv, .message = "illegal byte-order mark"sv}
            },

            // ----------------------
            // profiler
            // ----------------------

            {
                {.code = profiler::no_cpu_rtdscp_support, .locale = "en_US"sv},
                {.code = "T001"sv, .message = "CPU doesn't support RDTSCP instruction"sv}
            },

            {
                {.code = profiler::no_cpu_invariant_tsc_support, .locale = "en_US"sv},
                {.code = "T002"sv, .message = "CPU doesn't support invariant TSC"sv}
            },
        };

        g_error_system = memory::construct_with_allocator<error_system_t>(allocator, allocator, elements);

        g_error_system->pool_allocator = memory::construct_with_allocator<memory::frame_allocator_t<4096>>(
            g_error_system->allocator,
            g_error_system->allocator);

        g_error_system->pool = memory::construct_with_allocator<strings::pool_t>(
            g_error_system->allocator,
            g_error_system->pool_allocator);

        setlocale(LC_ALL, "");
        const auto lc = setlocale(LC_CTYPE, nullptr);
        string_t locale(lc, strlen(lc));
        const auto& parts = strings::string_to_list(locale, '.');
        if (parts.empty()) {
            g_error_system->locale = "en_US"sv;
        } else {
            g_error_system->locale = parts[0];
        }

        return true;
    }

    strings::pool_t& pool() {
        return *g_error_system->pool;
    }

    error_decl_t* find_decl(error_code_t code) {
        error_decl_key_t key{g_error_system->locale, code};
        return g_error_system->decls.find(key);
    }

}

template<>
uint64_t basecode::adt::hash_key(const basecode::errors::error_decl_key_t& key) {
    const auto composite_key = format::format("{}:{}"sv, key.locale, key.code);
    return hashing::murmur::hash64(composite_key.begin(), composite_key.size());
}
