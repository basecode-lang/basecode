#pragma once

#include <string>
#include <cerrno>
#include <climits>
#include <map>

namespace basecode {

    enum class token_types_t {
        unknown,
        left_square_bracket,
        right_square_bracket,
        left_paren,
        right_paren,
        left_curly_brace,
        right_curly_brace,
        line_comment,
        block_comment, //
        comma,
        assignment,
        equals,
        not_equals,
        greater_than,
        greater_than_equal,
        less_than,
        less_than_equal,
        logical_or,
        logical_and,
        plus,
        minus,
        asterisk,
        slash,
        caret,
        tilde,
        percent,
        bang,
        question,
        colon,
        semi_colon,
        pipe,
        ampersand,
        scope_operator,
        spread_operator,
        identifier,
        attribute,
        string_literal,
        character_literal,
        true_literal,
        false_literal,
        null_literal,
        empty_literal,
        none_literal,
        number_literal,
        namespace_literal,
        for_literal,
        in_literal,
        if_literal,
        else_if_literal,
        else_literal,
        alias_literal,
        extend_literal,
        break_literal,
        continue_literal,
        while_literal,
        cast_literal,
        read_only_literal,
        defer_literal,
        fn_literal,
        end_of_file
    };

    static inline std::map<token_types_t, std::string> s_type_to_name = {
        {token_types_t::unknown,                "unknown"},
        {token_types_t::left_square_bracket,    "left_square_bracket"},
        {token_types_t::right_square_bracket,   "right_square_bracket"},
        {token_types_t::left_paren,             "left_paren"},
        {token_types_t::right_paren,            "right_paren"},
        {token_types_t::left_curly_brace,       "left_curly_brace"},
        {token_types_t::right_curly_brace,      "right_curly_brace"},
        {token_types_t::line_comment,           "line_comment"},
        {token_types_t::block_comment,          "block_comment"},
        {token_types_t::comma,                  "comma"},
        {token_types_t::assignment,             "assignment"},
        {token_types_t::equals,                 "equals"},
        {token_types_t::not_equals,             "not_equals"},
        {token_types_t::greater_than,           "greater_than"},
        {token_types_t::greater_than_equal,     "greater_than_equal"},
        {token_types_t::less_than,              "less_than"},
        {token_types_t::less_than_equal,        "less_than_equal"},
        {token_types_t::logical_or,             "logical_or"},
        {token_types_t::logical_and,            "logical_and"},
        {token_types_t::plus,                   "plus"},
        {token_types_t::minus,                  "minus"},
        {token_types_t::asterisk,               "asterisk"},
        {token_types_t::slash,                  "slash"},
        {token_types_t::caret,                  "caret"},
        {token_types_t::tilde,                  "tilde"},
        {token_types_t::percent,                "percent"},
        {token_types_t::bang,                   "bang"},
        {token_types_t::question,               "question"},
        {token_types_t::colon,                  "colon"},
        {token_types_t::semi_colon,             "semi_colon"},
        {token_types_t::pipe,                   "pipe"},
        {token_types_t::ampersand,              "ampersand"},
        {token_types_t::scope_operator,         "scope_operator"},
        {token_types_t::spread_operator,        "spread_operator"},
        {token_types_t::identifier,             "identifier"},
        {token_types_t::attribute,              "attribute"},
        {token_types_t::string_literal,         "string_literal"},
        {token_types_t::character_literal,      "character_literal"},
        {token_types_t::true_literal,           "true_literal"},
        {token_types_t::false_literal,          "false_literal"},
        {token_types_t::null_literal,           "null_literal"},
        {token_types_t::empty_literal,          "empty_literal"},
        {token_types_t::none_literal,           "none_literal"},
        {token_types_t::number_literal,         "number_literal"},
        {token_types_t::namespace_literal,      "namespace_literal"},
        {token_types_t::for_literal,            "for_literal"},
        {token_types_t::in_literal,             "in_literal"},
        {token_types_t::if_literal,             "if_literal"},
        {token_types_t::else_if_literal,        "else_if_literal"},
        {token_types_t::else_literal,           "else_literal"},
        {token_types_t::alias_literal,          "alias_literal"},
        {token_types_t::extend_literal,         "extend_literal"},
        {token_types_t::break_literal,          "break_literal"},
        {token_types_t::continue_literal,       "continue_literal"},
        {token_types_t::while_literal,          "while_literal"},
        {token_types_t::cast_literal,           "cast_literal"},
        {token_types_t::read_only_literal,      "read_only_literal"},
        {token_types_t::defer_literal,          "defer_literal"},
        {token_types_t::fn_literal,             "fn_literal"},
        {token_types_t::end_of_file,            "end_of_file"},
    };

    enum class conversion_result {
        success,
        overflow,
        underflow,
        inconvertible
    };

    struct token_t {
        bool as_bool() const;

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
    };
};

