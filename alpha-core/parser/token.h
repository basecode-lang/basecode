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

#include <string>
#include <cerrno>
#include <climits>
#include <unordered_map>
#include <common/source_location.h>

namespace basecode::syntax {

    enum class token_types_t {
        invalid,
        plus,
        bang,
        pipe,
        minus,
        slash,
        caret,
        tilde,
        colon,
        comma,
        label,
        equals,
        period,
        percent,
        question,
        asterisk,
        exponent,
        ampersand,
        attribute,
        directive,
        less_than,
        raw_block,
        not_equals,
        left_paren,
        semi_colon,
        in_literal,
        if_literal,
        identifier,
        assignment,
        logical_or,
        logical_and,
        right_paren,
        for_literal,
        end_of_file,
        xor_literal,
        shl_literal,
        shr_literal,
        rol_literal,
        ror_literal,
        from_literal,
        proc_literal,
        with_literal,
        greater_than,
        line_comment,
        true_literal,
        enum_literal,
        null_literal,
        else_literal,
        cast_literal,
        false_literal,
        break_literal,
        while_literal,
        defer_literal,
        union_literal,
        block_comment,
        module_literal,
        struct_literal,
        number_literal,
        scope_operator,
        string_literal,
        return_literal,
        import_literal,
        less_than_equal,
        spread_operator,
        else_if_literal,
        left_curly_brace,
        continue_literal,
        right_curly_brace,
        character_literal,
        namespace_literal,
        transmute_literal,
        greater_than_equal,
        constant_assignment,
        left_square_bracket,
        right_square_bracket,
    };

    static inline std::unordered_map<token_types_t, std::string> s_type_to_name = {
        {token_types_t::invalid,                "invalid"},
        {token_types_t::plus,                   "plus"},
        {token_types_t::bang,                   "bang"},
        {token_types_t::pipe,                   "pipe"},
        {token_types_t::minus,                  "minus"},
        {token_types_t::label,                  "label"},
        {token_types_t::slash,                  "slash"},
        {token_types_t::caret,                  "caret"},
        {token_types_t::tilde,                  "tilde"},
        {token_types_t::comma,                  "comma"},
        {token_types_t::colon,                  "colon"},
        {token_types_t::equals,                 "equals"},
        {token_types_t::period,                 "period"},
        {token_types_t::percent,                "percent"},
        {token_types_t::question,               "question"},
        {token_types_t::asterisk,               "asterisk"},
        {token_types_t::exponent,               "exponent"},
        {token_types_t::less_than,              "less_than"},
        {token_types_t::raw_block,              "raw_block"},
        {token_types_t::ampersand,              "ampersand"},
        {token_types_t::attribute,              "attribute"},
        {token_types_t::directive,              "directive"},
        {token_types_t::identifier,             "identifier"},
        {token_types_t::not_equals,             "not_equals"},
        {token_types_t::assignment,             "assignment"},
        {token_types_t::left_paren,             "left_paren"},
        {token_types_t::logical_or,             "logical_or"},
        {token_types_t::semi_colon,             "semi_colon"},
        {token_types_t::in_literal,             "in_literal"},
        {token_types_t::if_literal,             "if_literal"},
        {token_types_t::for_literal,            "for_literal"},
        {token_types_t::end_of_file,            "end_of_file"},
        {token_types_t::right_paren,            "right_paren"},
        {token_types_t::logical_and,            "logical_and"},
        {token_types_t::xor_literal,            "xor_literal"},
        {token_types_t::shl_literal,            "shl_literal"},
        {token_types_t::shr_literal,            "shr_literal"},
        {token_types_t::rol_literal,            "rol_literal"},
        {token_types_t::ror_literal,            "ror_literal"},
        {token_types_t::from_literal,           "from_literal"},
        {token_types_t::proc_literal,           "proc_literal"},
        {token_types_t::null_literal,           "null_literal"},
        {token_types_t::with_literal,           "with_literal"},
        {token_types_t::cast_literal,           "cast_literal"},
        {token_types_t::else_literal,           "else_literal"},
        {token_types_t::line_comment,           "line_comment"},
        {token_types_t::greater_than,           "greater_than"},
        {token_types_t::enum_literal,           "enum_literal"},
        {token_types_t::true_literal,           "true_literal"},
        {token_types_t::false_literal,          "false_literal"},
        {token_types_t::break_literal,          "break_literal"},
        {token_types_t::while_literal,          "while_literal"},
        {token_types_t::defer_literal,          "defer_literal"},
        {token_types_t::union_literal,          "union_literal"},
        {token_types_t::block_comment,          "block_comment"},
        {token_types_t::module_literal,         "module_literal"},
        {token_types_t::struct_literal,         "struct_literal"},
        {token_types_t::return_literal,         "return_literal"},
        {token_types_t::number_literal,         "number_literal"},
        {token_types_t::scope_operator,         "scope_operator"},
        {token_types_t::string_literal,         "string_literal"},
        {token_types_t::spread_operator,        "spread_operator"},
        {token_types_t::import_literal,         "import_literal"},
        {token_types_t::less_than_equal,        "less_than_equal"},
        {token_types_t::else_if_literal,        "else_if_literal"},
        {token_types_t::left_curly_brace,       "left_curly_brace"},
        {token_types_t::continue_literal,       "continue_literal"},
        {token_types_t::right_curly_brace,      "right_curly_brace"},
        {token_types_t::character_literal,      "character_literal"},
        {token_types_t::namespace_literal,      "namespace_literal"},
        {token_types_t::transmute_literal,      "transmute_literal"},
        {token_types_t::greater_than_equal,     "greater_than_equal"},
        {token_types_t::constant_assignment,    "constant_assignment"},
        {token_types_t::left_square_bracket,    "left_square_bracket"},
        {token_types_t::right_square_bracket,   "right_square_bracket"},
    };

    enum class conversion_result_t {
        success,
        overflow,
        underflow,
        inconvertible
    };

    enum class number_types_t {
        none,
        integer,
        floating_point,
    };

    struct token_t {
        bool as_bool() const;

        bool is_label() const;

        bool is_signed() const;

        bool is_boolean() const;

        bool is_numeric() const;

        std::string name() const;

        bool is_line_comment() const;

        bool is_block_comment() const;

        conversion_result_t parse(double& out) const;

        conversion_result_t parse(int64_t& out) const;

        conversion_result_t parse(uint64_t& out) const;

        token_types_t type = token_types_t::invalid;
        std::string value {};
        uint8_t radix = 10;
        common::source_location location {};
        number_types_t number_type = number_types_t::none;
    };

    static inline token_t s_invalid = {
        .type = token_types_t::invalid,
        .value = ""
    };

    static inline token_t s_raw_block = {
        .type = token_types_t::raw_block,
        .value = "{{"
    };

    static inline token_t s_end_of_file = {
        .type = token_types_t::end_of_file,
        .value = ""
    };

    static inline token_t s_block_comment = {
        .type = token_types_t::block_comment,
        .value = "/*"
    };

    static inline token_t s_proc_literal = {
        .type = token_types_t::proc_literal,
        .value = "proc"
    };

    static inline token_t s_if_literal = {
        .type = token_types_t::if_literal,
        .value = "if"
    };

    static inline token_t s_in_literal = {
        .type = token_types_t::in_literal,
        .value = "in"
    };

    static inline token_t s_for_literal = {
        .type = token_types_t::for_literal,
        .value = "for"
    };

    static inline token_t s_exponent_literal = {
        .type = token_types_t::exponent,
        .value = "**"
    };

    static inline token_t s_xor_literal = {
        .type = token_types_t::xor_literal,
        .value = "xor"
    };

    static inline token_t s_shl_literal = {
        .type = token_types_t::shl_literal,
        .value = "shl"
    };

    static inline token_t s_shr_literal = {
        .type = token_types_t::shr_literal,
        .value = "shr"
    };

    static inline token_t s_rol_literal = {
        .type = token_types_t::rol_literal,
        .value = "rol"
    };

    static inline token_t s_ror_literal = {
        .type = token_types_t::ror_literal,
        .value = "ror"
    };

    static inline token_t s_else_literal = {
        .type = token_types_t::else_literal,
        .value = "else"
    };

    static inline token_t s_bang_literal = {
        .type = token_types_t::bang,
        .value = "!"
    };

    static inline token_t s_plus_literal = {
        .type = token_types_t::plus,
        .value = "+"
    };

    static inline token_t s_pipe_literal = {
        .type = token_types_t::pipe,
        .value = "|"
    };

    static inline token_t s_colon_literal = {
        .type = token_types_t::colon,
        .value = ":"
    };

    static inline token_t s_enum_literal = {
        .type = token_types_t::enum_literal,
        .value = "enum"
    };

    static inline token_t s_cast_literal = {
        .type = token_types_t::cast_literal,
        .value = "cast"
    };

    static inline token_t s_null_literal = {
        .type = token_types_t::null_literal,
        .value = "null"
    };

    static inline token_t s_with_literal = {
        .type = token_types_t::with_literal,
        .value = "with"
    };

    static inline token_t s_from_literal = {
        .type = token_types_t::from_literal,
        .value = "from"
    };

    static inline token_t s_true_literal = {
        .type = token_types_t::true_literal,
        .value = "true"
    };

    static inline token_t s_minus_literal = {
        .type = token_types_t::minus,
        .value = "-"
    };

    static inline token_t s_slash_literal = {
        .type = token_types_t::slash,
        .value = "/"
    };

    static inline token_t s_comma_literal = {
        .type = token_types_t::comma,
        .value = ","
    };

    static inline token_t s_defer_literal = {
        .type = token_types_t::defer_literal,
        .value = "defer"
    };

    static inline token_t s_module_literal = {
        .type = token_types_t::module_literal,
        .value = "module"
    };

    static inline token_t s_break_literal = {
        .type = token_types_t::break_literal,
        .value = "break"
    };

    static inline token_t s_false_literal = {
        .type = token_types_t::false_literal,
        .value = "false"
    };

    static inline token_t s_while_literal = {
        .type = token_types_t::while_literal,
        .value = "while"
    };

    static inline token_t s_union_literal = {
        .type = token_types_t::union_literal,
        .value = "union"
    };

    static inline token_t s_caret_literal = {
        .type = token_types_t::caret,
        .value = "^"
    };

    static inline token_t s_tilde_literal = {
        .type = token_types_t::tilde,
        .value = "~"
    };

    static inline token_t s_import_literal = {
        .type = token_types_t::import_literal,
        .value = "import"
    };

    static inline token_t s_period_literal = {
        .type = token_types_t::period,
        .value = "."
    };

    static inline token_t s_struct_literal = {
        .type = token_types_t::struct_literal,
        .value = "struct"
    };

    static inline token_t s_return_literal = {
        .type = token_types_t::return_literal,
        .value = "return"
    };

    static inline token_t s_else_if_literal = {
        .type = token_types_t::else_if_literal,
        .value = "else if"
    };

    static inline token_t s_percent_literal = {
        .type = token_types_t::percent,
        .value = "%"
    };

    static inline token_t s_continue_literal = {
        .type = token_types_t::continue_literal,
        .value = "continue"
    };

    static inline token_t s_asterisk_literal = {
        .type = token_types_t::asterisk,
        .value = "*"
    };

    static inline token_t s_question_literal = {
        .type = token_types_t::question,
        .value = "?"
    };

    static inline token_t s_transmute_literal = {
        .type = token_types_t::transmute_literal,
        .value = "transmute"
    };

    static inline token_t s_namespace_literal = {
        .type = token_types_t::namespace_literal,
        .value = "ns"
    };

    static inline token_t s_assignment_literal = {
        .type = token_types_t::assignment,
        .value = ":="
    };

    static inline token_t s_ampersand_literal = {
        .type = token_types_t::ampersand,
        .value = "&"
    };

    static inline token_t s_left_paren_literal = {
        .type = token_types_t::left_paren,
        .value = "("
    };

    static inline token_t s_logical_or_literal = {
        .type = token_types_t::logical_or,
        .value = "||"
    };

    static inline token_t s_semi_colon_literal = {
        .type = token_types_t::semi_colon,
        .value = ";"
    };

    static inline token_t s_right_paren_literal = {
        .type = token_types_t::right_paren,
        .value = ")"
    };

    static inline token_t s_logical_and_literal = {
        .type = token_types_t::logical_and,
        .value = "&&"
    };

    static inline token_t s_equals_literal = {
        .type = token_types_t::equals,
        .value = "=="
    };

    static inline token_t s_not_equals_literal = {
        .type = token_types_t::not_equals,
        .value = "!="
    };

    static inline token_t s_greater_than_literal = {
        .type = token_types_t::greater_than,
        .value = ">"
    };

    static inline token_t s_less_than_literal = {
        .type = token_types_t::less_than,
        .value = "<"
    };

    static inline token_t s_greater_than_equal_literal = {
        .type = token_types_t::greater_than_equal,
        .value = ">="
    };

    static inline token_t s_less_than_equal_literal = {
        .type = token_types_t::less_than,
        .value = "<="
    };

    static inline token_t s_scope_operator_literal = {
        .type = token_types_t::scope_operator,
        .value = "::"
    };

    static inline token_t s_spread_operator_literal = {
        .type = token_types_t::spread_operator,
        .value = "..."
    };

    static inline token_t s_left_curly_brace_literal = {
        .type = token_types_t::left_curly_brace,
        .value = "{"
    };

    static inline token_t s_right_curly_brace_literal = {
        .type = token_types_t::right_curly_brace,
        .value = "}"
    };

    static inline token_t s_constant_assignment_literal = {
        .type = token_types_t::constant_assignment,
        .value = "::="
    };

    static inline token_t s_left_square_bracket_literal = {
        .type = token_types_t::left_square_bracket,
        .value = "["
    };

    static inline token_t s_right_square_bracket_literal = {
        .type = token_types_t::right_square_bracket,
        .value = "]"
    };

};