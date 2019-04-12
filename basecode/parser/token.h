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
#include <string_view>
#include <unordered_map>
#include <common/source_location.h>

namespace basecode::syntax {

    using namespace std::literals;

    enum class token_type_t {
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
        nil_literal,
        case_literal,
        from_literal,
        proc_literal,
        with_literal,
        greater_than,
        line_comment,
        true_literal,
        enum_literal,
        else_literal,
        false_literal,
        break_literal,
        while_literal,
        defer_literal,
        union_literal,
        yield_literal,
        block_comment,
        module_literal,
        struct_literal,
        number_literal,
        scope_operator,
        string_literal,
        return_literal,
        import_literal,
        lambda_literal,
        switch_literal,
        less_than_equal,
        spread_operator,
        else_if_literal,
        left_curly_brace,
        continue_literal,
        right_curly_brace,
        character_literal,
        namespace_literal,
        value_sink_literal,
        key_value_operator,
        greater_than_equal,
        plus_equal_literal,
        fallthrough_literal,
        minus_equal_literal,
        constant_assignment,
        left_square_bracket,
        divide_equal_literal,
        right_square_bracket,
        modulus_equal_literal,
        control_flow_operator,
        multiply_equal_literal,
        type_tagged_identifier,
        binary_or_equal_literal,
        binary_not_equal_literal,
        binary_and_equal_literal,
    };

    static inline std::unordered_map<token_type_t, std::string_view> s_type_to_name = {
        {token_type_t::invalid,                    "invalid"sv},
        {token_type_t::plus,                       "plus"sv},
        {token_type_t::bang,                       "bang"sv},
        {token_type_t::pipe,                       "pipe"sv},
        {token_type_t::minus,                      "minus"sv},
        {token_type_t::label,                      "label"sv},
        {token_type_t::slash,                      "slash"sv},
        {token_type_t::caret,                      "caret"sv},
        {token_type_t::tilde,                      "tilde"sv},
        {token_type_t::comma,                      "comma"sv},
        {token_type_t::colon,                      "colon"sv},
        {token_type_t::equals,                     "equals"sv},
        {token_type_t::period,                     "period"sv},
        {token_type_t::percent,                    "percent"sv},
        {token_type_t::question,                   "question"sv},
        {token_type_t::asterisk,                   "asterisk"sv},
        {token_type_t::exponent,                   "exponent"sv},
        {token_type_t::less_than,                  "less_than"sv},
        {token_type_t::raw_block,                  "raw_block"sv},
        {token_type_t::ampersand,                  "ampersand"sv},
        {token_type_t::attribute,                  "attribute"sv},
        {token_type_t::directive,                  "directive"sv},
        {token_type_t::identifier,                 "identifier"sv},
        {token_type_t::not_equals,                 "not_equals"sv},
        {token_type_t::assignment,                 "assignment"sv},
        {token_type_t::left_paren,                 "left_paren"sv},
        {token_type_t::logical_or,                 "logical_or"sv},
        {token_type_t::semi_colon,                 "semi_colon"sv},
        {token_type_t::in_literal,                 "in_literal"sv},
        {token_type_t::if_literal,                 "if_literal"sv},
        {token_type_t::nil_literal,                "nil_literal"sv},
        {token_type_t::for_literal,                "for_literal"sv},
        {token_type_t::end_of_file,                "end_of_file"sv},
        {token_type_t::right_paren,                "right_paren"sv},
        {token_type_t::logical_and,                "logical_and"sv},
        {token_type_t::xor_literal,                "xor_literal"sv},
        {token_type_t::shl_literal,                "shl_literal"sv},
        {token_type_t::shr_literal,                "shr_literal"sv},
        {token_type_t::rol_literal,                "rol_literal"sv},
        {token_type_t::ror_literal,                "ror_literal"sv},
        {token_type_t::case_literal,               "case_literal"sv},
        {token_type_t::from_literal,               "from_literal"sv},
        {token_type_t::proc_literal,               "proc_literal"sv},
        {token_type_t::with_literal,               "with_literal"sv},
        {token_type_t::else_literal,               "else_literal"sv},
        {token_type_t::line_comment,               "line_comment"sv},
        {token_type_t::greater_than,               "greater_than"sv},
        {token_type_t::enum_literal,               "enum_literal"sv},
        {token_type_t::true_literal,               "true_literal"sv},
        {token_type_t::yield_literal,              "yield_literal"sv},
        {token_type_t::false_literal,              "false_literal"sv},
        {token_type_t::break_literal,              "break_literal"sv},
        {token_type_t::while_literal,              "while_literal"sv},
        {token_type_t::defer_literal,              "defer_literal"sv},
        {token_type_t::union_literal,              "union_literal"sv},
        {token_type_t::block_comment,              "block_comment"sv},
        {token_type_t::lambda_literal,             "lambda_literal"sv},
        {token_type_t::module_literal,             "module_literal"sv},
        {token_type_t::struct_literal,             "struct_literal"sv},
        {token_type_t::return_literal,             "return_literal"sv},
        {token_type_t::number_literal,             "number_literal"sv},
        {token_type_t::scope_operator,             "scope_operator"sv},
        {token_type_t::string_literal,             "string_literal"sv},
        {token_type_t::switch_literal,             "switch_literal"sv},
        {token_type_t::spread_operator,            "spread_operator"sv},
        {token_type_t::import_literal,             "import_literal"sv},
        {token_type_t::less_than_equal,            "less_than_equal"sv},
        {token_type_t::else_if_literal,            "else_if_literal"sv},
        {token_type_t::left_curly_brace,           "left_curly_brace"sv},
        {token_type_t::continue_literal,           "continue_literal"sv},
        {token_type_t::right_curly_brace,          "right_curly_brace"sv},
        {token_type_t::character_literal,          "character_literal"sv},
        {token_type_t::namespace_literal,          "namespace_literal"sv},
        {token_type_t::key_value_operator,         "key_value_operator"sv},
        {token_type_t::plus_equal_literal,         "plus_equal_literal"sv},
        {token_type_t::greater_than_equal,         "greater_than_equal"sv},
        {token_type_t::value_sink_literal,         "value_sink_literal"sv},
        {token_type_t::fallthrough_literal,        "fallthrough_literal"sv},
        {token_type_t::minus_equal_literal,        "minus_equal_literal"sv},
        {token_type_t::constant_assignment,        "constant_assignment"sv},
        {token_type_t::left_square_bracket,        "left_square_bracket"sv},
        {token_type_t::right_square_bracket,       "right_square_bracket"sv},
        {token_type_t::divide_equal_literal,       "divide_equal_literal"sv},
        {token_type_t::control_flow_operator,      "control_flow_operator"sv},
        {token_type_t::modulus_equal_literal,      "modulus_equal_literal"sv},
        {token_type_t::multiply_equal_literal,     "multiply_equal_literal"sv},
        {token_type_t::type_tagged_identifier,     "type_tagged_identifier"sv},
        {token_type_t::binary_or_equal_literal,    "binary_or_equal_literal"sv},
        {token_type_t::binary_and_equal_literal,   "binary_and_equal_literal"sv},
        {token_type_t::binary_not_equal_literal,   "binary_not_equal_literal"sv},
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

        std::string_view name() const;

        bool is_line_comment() const;

        bool is_block_comment() const;

        conversion_result_t parse(double& out) const;

        conversion_result_t parse(int64_t& out) const;

        conversion_result_t parse(uint64_t& out) const;

        uint8_t radix = 10;
        std::string_view value {};
        common::source_location location {};
        token_type_t type = token_type_t::invalid;
        number_types_t number_type = number_types_t::none;
    };

    static inline token_t s_invalid = {
        .type = token_type_t::invalid,
    };

    static inline token_t s_raw_block = {
        .value = "{{"sv,
        .type = token_type_t::raw_block,
    };

    static inline token_t s_in_literal = {
        .value = "in"sv,
        .type = token_type_t::in_literal,
    };

    static inline token_t s_if_literal = {
        .value = "if"sv,
        .type = token_type_t::if_literal,
    };

    static inline token_t s_nil_literal = {
        .value = "nil"sv,
        .type = token_type_t::nil_literal,
    };

    static inline token_t s_xor_literal = {
        .value = "xor"sv,
        .type = token_type_t::xor_literal,
    };

    static inline token_t s_shl_literal = {
        .value = "shl"sv,
        .type = token_type_t::shl_literal,
    };

    static inline token_t s_shr_literal = {
        .value = "shr"sv,
        .type = token_type_t::shr_literal,
    };

    static inline token_t s_rol_literal = {
        .value = "rol"sv,
        .type = token_type_t::rol_literal,
    };

    static inline token_t s_ror_literal = {
        .value = "ror"sv,
        .type = token_type_t::ror_literal,
    };

    static inline token_t s_for_literal = {
        .value = "for"sv,
        .type = token_type_t::for_literal,
    };

    static inline token_t s_end_of_file = {
        .type = token_type_t::end_of_file,
    };

    static inline token_t s_proc_literal = {
        .value = "proc"sv,
        .type = token_type_t::proc_literal,
    };

    static inline token_t s_else_literal = {
        .value = "else"sv,
        .type = token_type_t::else_literal,
    };

    static inline token_t s_bang_literal = {
        .value = "!"sv,
        .type = token_type_t::bang,
    };

    static inline token_t s_plus_literal = {
        .value = "+"sv,
        .type = token_type_t::plus,
    };

    static inline token_t s_pipe_literal = {
        .value = "|"sv,
        .type = token_type_t::pipe,
    };

    static inline token_t s_enum_literal = {
        .type = token_type_t::enum_literal,
        .value = "enum"
    };

    static inline token_t s_with_literal = {
        .value = "with"sv,
        .type = token_type_t::with_literal,
    };

    static inline token_t s_from_literal = {
        .value = "from"sv,
        .type = token_type_t::from_literal,
    };

    static inline token_t s_true_literal = {
        .value = "true"sv,
        .type = token_type_t::true_literal,
    };

    static inline token_t s_case_literal = {
        .value = "case"sv,
        .type = token_type_t::case_literal,
    };

    static inline token_t s_block_comment = {
        .value = "/*"sv,
        .type = token_type_t::block_comment,
    };

    static inline token_t s_yield_literal = {
        .value = "yield"sv,
        .type = token_type_t::yield_literal,
    };

    static inline token_t s_colon_literal = {
        .value = ":"sv,
        .type = token_type_t::colon,
    };

    static inline token_t s_minus_literal = {
        .value = "-"sv,
        .type = token_type_t::minus,
    };

    static inline token_t s_slash_literal = {
        .value = "/"sv,
        .type = token_type_t::slash,
    };

    static inline token_t s_comma_literal = {
        .value = ","sv,
        .type = token_type_t::comma,
    };

    static inline token_t s_defer_literal = {
        .value = "defer"sv,
        .type = token_type_t::defer_literal,
    };

    static inline token_t s_break_literal = {
        .value = "break"sv,
        .type = token_type_t::break_literal,
    };

    static inline token_t s_false_literal = {
        .value = "false"sv,
        .type = token_type_t::false_literal,
    };

    static inline token_t s_while_literal = {
        .value = "while"sv,
        .type = token_type_t::while_literal,
    };

    static inline token_t s_union_literal = {
        .value = "union"sv,
        .type = token_type_t::union_literal,
    };

    static inline token_t s_caret_literal = {
        .value = "^"sv,
        .type = token_type_t::caret,
    };

    static inline token_t s_tilde_literal = {
        .value = "~"sv,
        .type = token_type_t::tilde,
    };

    static inline token_t s_module_literal = {
        .value = "module"sv,
        .type = token_type_t::module_literal,
    };

    static inline token_t s_import_literal = {
        .value = "import"sv,
        .type = token_type_t::import_literal,
    };

    static inline token_t s_period_literal = {
        .value = "."sv,
        .type = token_type_t::period,
    };

    static inline token_t s_struct_literal = {
        .value = "struct"sv,
        .type = token_type_t::struct_literal,
    };

    static inline token_t s_return_literal = {
        .value = "return"sv,
        .type = token_type_t::return_literal,
    };

    static inline token_t s_switch_literal = {
        .value = "switch"sv,
        .type = token_type_t::switch_literal,
    };

    static inline token_t s_equals_literal = {
        .value = "=="sv,
        .type = token_type_t::equals,
    };

    static inline token_t s_else_if_literal = {
        .value = "else if"sv,
        .type = token_type_t::else_if_literal,
    };

    static inline token_t s_percent_literal = {
        .value = "%"sv,
        .type = token_type_t::percent,
    };

    static inline token_t s_exponent_literal = {
        .value = "**"sv,
        .type = token_type_t::exponent,
    };

    static inline token_t s_continue_literal = {
        .value = "continue"sv,
        .type = token_type_t::continue_literal,
    };

    static inline token_t s_asterisk_literal = {
        .value = "*"sv,
        .type = token_type_t::asterisk,
    };

    static inline token_t s_question_literal = {
        .value = "?"sv,
        .type = token_type_t::question,
    };

    static inline token_t s_namespace_literal = {
        .value = "ns"sv,
        .type = token_type_t::namespace_literal,
    };

    static inline token_t s_ampersand_literal = {
        .value = "&"sv,
        .type = token_type_t::ampersand,
    };

    static inline token_t s_less_than_literal = {
        .value = "<"sv,
        .type = token_type_t::less_than,
    };

    static inline token_t s_left_paren_literal = {
        .value = "("sv,
        .type = token_type_t::left_paren,
    };

    static inline token_t s_logical_or_literal = {
        .value = "||"sv,
        .type = token_type_t::logical_or,
    };

    static inline token_t s_semi_colon_literal = {
        .value = ";"sv,
        .type = token_type_t::semi_colon,
    };

    static inline token_t s_assignment_literal = {
        .value = ":="sv,
        .type = token_type_t::assignment,
    };

    static inline token_t s_key_value_operator = {
        .value = ":="sv,
        .type = token_type_t::key_value_operator,
    };

    static inline token_t s_plus_equal_literal = {
        .value = "+:="sv,
        .type = token_type_t::plus_equal_literal,
    };

    static inline token_t s_not_equals_literal = {
        .value = "!="sv,
        .type = token_type_t::not_equals,
    };

    static inline token_t s_value_sink_literal = {
        .value = "_"sv,
        .type = token_type_t::value_sink_literal,
    };

    static inline token_t s_right_paren_literal = {
        .value = ")"sv,
        .type = token_type_t::right_paren,
    };

    static inline token_t s_logical_and_literal = {
        .type = token_type_t::logical_and,
        .value = "&&"
    };

    static inline token_t s_minus_equal_literal = {
        .value = "-:="sv,
        .type = token_type_t::minus_equal_literal,
    };

    static inline token_t s_fallthrough_literal = {
        .value = "fallthrough"sv,
        .type = token_type_t::fallthrough_literal,
    };

    static inline token_t s_greater_than_literal = {
        .value = ">"sv,
        .type = token_type_t::greater_than,
    };

    static inline token_t s_divide_equal_literal = {
        .value = "/:="sv,
        .type = token_type_t::divide_equal_literal,
    };

    static inline token_t s_modulus_equal_literal = {
        .value = "%:="sv,
        .type = token_type_t::modulus_equal_literal,
    };

    static inline token_t s_control_flow_operator = {
        .value = "=>"sv,
        .type = token_type_t::control_flow_operator,
    };

    static inline token_t s_multiply_equal_literal = {
        .value = "*:="sv,
        .type = token_type_t::multiply_equal_literal,
    };

    static inline token_t s_scope_operator_literal = {
        .value = "::"sv,
        .type = token_type_t::scope_operator,
    };

    static inline token_t s_spread_operator_literal = {
        .value = "..."sv,
        .type = token_type_t::spread_operator,
    };

    static inline token_t s_less_than_equal_literal = {
        .value = "<="sv,
        .type = token_type_t::less_than_equal,
    };

    static inline token_t s_binary_or_equal_literal = {
        .value = "|:="sv,
        .type = token_type_t::binary_or_equal_literal,
    };

    static inline token_t s_binary_and_equal_literal = {
        .value = "&:="sv,
        .type = token_type_t::binary_and_equal_literal,
    };

    static inline token_t s_binary_not_equal_literal = {
        .value = "~:="sv,
        .type = token_type_t::binary_not_equal_literal,
    };

    static inline token_t s_left_curly_brace_literal = {
        .value = "{"sv,
        .type = token_type_t::left_curly_brace,
    };

    static inline token_t s_right_curly_brace_literal = {
        .value = "}"sv,
        .type = token_type_t::right_curly_brace,
    };

    static inline token_t s_greater_than_equal_literal = {
        .value = ">="sv,
        .type = token_type_t::greater_than_equal,
    };

    static inline token_t s_constant_assignment_literal = {
        .value = "::"sv,
        .type = token_type_t::constant_assignment,
    };

    static inline token_t s_left_square_bracket_literal = {
        .value = "["sv,
        .type = token_type_t::left_square_bracket,
    };

    static inline token_t s_right_square_bracket_literal = {
        .value = "]"sv,
        .type = token_type_t::right_square_bracket,
    };

    static inline token_t extract_non_assign_operator(const token_t& token) {
        switch (token.type) {
            case token_type_t::plus_equal_literal:
                return s_plus_literal;
            case token_type_t::minus_equal_literal:
                return s_minus_literal;
            case token_type_t::divide_equal_literal:
                return s_slash_literal;
            case token_type_t::modulus_equal_literal:
                return s_percent_literal;
            case token_type_t::multiply_equal_literal:
                return s_asterisk_literal;
            case token_type_t::binary_or_equal_literal:
                return s_pipe_literal;
            case token_type_t::binary_and_equal_literal:
                return s_ampersand_literal;
            case token_type_t::binary_not_equal_literal:
                return s_tilde_literal;
            default:
                return token;
        }
    }

}