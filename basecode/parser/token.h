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
#include <common/id_pool.h>
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

    static inline std::string_view token_type_to_name(token_type_t type) {
        auto it = s_type_to_name.find(type);
        if (it == s_type_to_name.end())
            return "unknown"sv;
        return it->second;
    }

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
        token_t() = default;

        token_t(
            common::id_t id,
            token_type_t type) : id(id),
                                 type(type) {
        }

        token_t(
            common::id_t id,
            token_type_t type,
            std::string_view value) : id(id),
                                      value(value),
                                      type(type) {
        }

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

        common::id_t id {};
        uint8_t radix = 10;
        std::string_view value {};
        common::source_location location {};
        token_type_t type = token_type_t::invalid;
        number_types_t number_type = number_types_t::none;
    };

    using token_list_t = std::vector<token_t>;

    static inline std::pair<token_type_t, bool> extract_non_assign_operator(token_t* token) {
        switch (token->type) {
            case token_type_t::plus_equal_literal:
                return std::make_pair(token_type_t::plus, true);
            case token_type_t::minus_equal_literal:
                return std::make_pair(token_type_t::minus, true);
            case token_type_t::divide_equal_literal:
                return std::make_pair(token_type_t::slash, true);
            case token_type_t::modulus_equal_literal:
                return std::make_pair(token_type_t::percent, true);
            case token_type_t::multiply_equal_literal:
                return std::make_pair(token_type_t::asterisk, true);
            case token_type_t::binary_or_equal_literal:
                return std::make_pair(token_type_t::pipe, true);
            case token_type_t::binary_and_equal_literal:
                return std::make_pair(token_type_t::ampersand, true);
            case token_type_t::binary_not_equal_literal:
                return std::make_pair(token_type_t::tilde, true);
            default:
                return std::make_pair(token->type, false);
        }
    }

}