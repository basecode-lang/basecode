// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <string>
#include <cerrno>
#include <climits>
#include <unordered_map>

namespace basecode::syntax {

    enum class token_types_t {
        unknown,
        plus,
        bang,
        pipe,
        minus,
        slash,
        caret,
        tilde,
        colon,
        comma,
        equals,
        period,
        percent,
        question,
        asterisk,
        ampersand,
        attribute,
        directive,
        less_than,
        not_equals,
        left_paren,
        semi_colon,
        in_literal,
        if_literal,
        identifier,
        assignment,
        logical_or,
        fn_literal,
        logical_and,
        right_paren,
        for_literal,
        end_of_file,
        with_literal,
        greater_than,
        none_literal,
        line_comment,
        true_literal,
        enum_literal,
        null_literal,
        else_literal,
        cast_literal,
        block_comment, //
        false_literal,
        empty_literal,
        alias_literal,
        break_literal,
        while_literal,
        defer_literal,
        union_literal,
        struct_literal,
        number_literal,
        scope_operator,
        string_literal,
        return_literal,
        less_than_equal,
        spread_operator,
        else_if_literal,
        left_curly_brace,
        continue_literal,
        constant_literal,
        right_curly_brace,
        character_literal,
        namespace_literal,
        greater_than_equal,
        left_square_bracket,
        right_square_bracket,
    };

    static inline std::unordered_map<token_types_t, std::string> s_type_to_name = {
        {token_types_t::unknown,                "unknown"},
        {token_types_t::plus,                   "plus"},
        {token_types_t::bang,                   "bang"},
        {token_types_t::pipe,                   "pipe"},
        {token_types_t::minus,                  "minus"},
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
        {token_types_t::less_than,              "less_than"},
        {token_types_t::ampersand,              "ampersand"},
        {token_types_t::attribute,              "attribute"},
        {token_types_t::directive,              "directive"},
        {token_types_t::fn_literal,             "fn_literal"},
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
        {token_types_t::null_literal,           "null_literal"},
        {token_types_t::none_literal,           "none_literal"},
        {token_types_t::with_literal,           "with_literal"},
        {token_types_t::cast_literal,           "cast_literal"},
        {token_types_t::else_literal,           "else_literal"},
        {token_types_t::line_comment,           "line_comment"},
        {token_types_t::greater_than,           "greater_than"},
        {token_types_t::enum_literal,           "enum_literal"},
        {token_types_t::true_literal,           "true_literal"},
        {token_types_t::empty_literal,          "empty_literal"},
        {token_types_t::block_comment,          "block_comment"},
        {token_types_t::false_literal,          "false_literal"},
        {token_types_t::alias_literal,          "alias_literal"},
        {token_types_t::break_literal,          "break_literal"},
        {token_types_t::while_literal,          "while_literal"},
        {token_types_t::defer_literal,          "defer_literal"},
        {token_types_t::union_literal,          "union_literal"},
        {token_types_t::struct_literal,         "struct_literal"},
        {token_types_t::return_literal,         "return_literal"},
        {token_types_t::number_literal,         "number_literal"},
        {token_types_t::scope_operator,         "scope_operator"},
        {token_types_t::string_literal,         "string_literal"},
        {token_types_t::spread_operator,        "spread_operator"},
        {token_types_t::less_than_equal,        "less_than_equal"},
        {token_types_t::else_if_literal,        "else_if_literal"},
        {token_types_t::left_curly_brace,       "left_curly_brace"},
        {token_types_t::continue_literal,       "continue_literal"},
        {token_types_t::constant_literal,       "constant_literal"},
        {token_types_t::right_curly_brace,      "right_curly_brace"},
        {token_types_t::character_literal,      "character_literal"},
        {token_types_t::namespace_literal,      "namespace_literal"},
        {token_types_t::greater_than_equal,     "greater_than_equal"},
        {token_types_t::left_square_bracket,    "left_square_bracket"},
        {token_types_t::right_square_bracket,   "right_square_bracket"},
    };

    enum class conversion_result {
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

        bool is_comment() const;

        bool is_boolean() const;

        bool is_numeric() const;

        std::string name() const;

        conversion_result parse(int64_t& out) const;

        conversion_result parse(uint64_t& out) const;

        token_types_t type = token_types_t::unknown;
        std::string value {};
        uint8_t radix = 10;
        uint32_t line = 0;
        uint32_t column = 0;
        number_types_t number_type = number_types_t::none;
    };

    static inline token_t s_fn_literal = {
        .type = token_types_t::fn_literal,
        .value = "fn"
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

    static inline token_t s_none_literal = {
        .type = token_types_t::none_literal,
        .value = "none"
    };

    static inline token_t s_with_literal = {
        .type = token_types_t::with_literal,
        .value = "with"
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

    static inline token_t s_alias_literal = {
        .type = token_types_t::alias_literal,
        .value = "alias"
    };

    static inline token_t s_empty_literal = {
        .type = token_types_t::empty_literal,
        .value = "empty"
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

    static inline token_t s_constant_literal = {
        .type = token_types_t::constant_literal,
        .value = "constant"
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

    static inline token_t s_left_square_bracket_literal = {
        .type = token_types_t::left_square_bracket,
        .value = "["
    };

    static inline token_t s_right_square_bracket_literal = {
        .type = token_types_t::right_square_bracket,
        .value = "]"
    };

};