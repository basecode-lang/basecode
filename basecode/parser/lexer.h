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

#include <set>
#include <map>
#include <vector>
#include <istream>
#include <functional>
#include <common/rune.h>
#include <common/source_file.h>
#include "token.h"

namespace basecode::syntax {

    class lexer {
    public:
        using lexer_case_callable = std::function<bool (lexer*, common::result&)>;

        explicit lexer(common::source_file* source_file);

        bool has_next() const;

        bool next(token_t& token);

        bool tokenize(common::result& r);

    private:
        bool plus(common::result& r);

        bool bang(common::result& r);

        bool minus(common::result& r);

        bool comma(common::result& r);

        bool slash(common::result& r);

        bool caret(common::result& r);

        bool tilde(common::result& r);

        bool colon(common::result& r);

        bool label(common::result& r);

        bool period(common::result& r);

        bool spread(common::result& r);

        bool percent(common::result& r);

        bool asterisk(common::result& r);

        bool question(common::result& r);

        bool exponent(common::result& r);

        bool attribute(common::result& r);

        bool directive(common::result& r);

        bool raw_block(common::result& r);

        bool in_literal(common::result& r);

        bool if_literal(common::result& r);

        bool left_paren(common::result& r);

        bool identifier(common::result& r);

        bool assignment(common::result& r);

        bool ns_literal(common::result& r);

        bool xor_literal(common::result& r);

        bool shl_literal(common::result& r);

        bool shr_literal(common::result& r);

        bool rol_literal(common::result& r);

        bool ror_literal(common::result& r);

        bool for_literal(common::result& r);

        bool right_paren(common::result& r);

        bool nil_literal(common::result& r);

        bool case_literal(common::result& r);

        bool proc_literal(common::result& r);

        bool enum_literal(common::result& r);

        bool else_literal(common::result& r);

        bool line_comment(common::result& r);

        bool pipe_literal(common::result& r);

        bool true_literal(common::result& r);

        bool with_literal(common::result& r);

        bool from_literal(common::result& r);

        bool block_comment(common::result& r);

        bool false_literal(common::result& r);

        bool defer_literal(common::result& r);

        bool break_literal(common::result& r);

        bool while_literal(common::result& r);

        bool union_literal(common::result& r);

        bool yield_literal(common::result& r);

        bool module_literal(common::result& r);

        bool struct_literal(common::result& r);

        bool return_literal(common::result& r);

        bool number_literal(common::result& r);

        bool scope_operator(common::result& r);

        bool string_literal(common::result& r);

        bool import_literal(common::result& r);

        bool lambda_literal(common::result& r);

        bool switch_literal(common::result& r);

        bool line_terminator(common::result& r);

        bool equals_operator(common::result& r);

        bool else_if_literal(common::result& r);

        bool left_curly_brace(common::result& r);

        bool continue_literal(common::result& r);

        bool right_curly_brace(common::result& r);

        bool ampersand_literal(common::result& r);

        bool character_literal(common::result& r);

        bool value_sink_literal(common::result& r);

        bool less_than_operator(common::result& r);

        bool fallthrough_literal(common::result& r);

        bool not_equals_operator(common::result& r);

        bool left_square_bracket(common::result& r);

        bool logical_or_operator(common::result& r);

        bool constant_assignment(common::result& r);

        bool plus_equal_operator(common::result& r);

        bool logical_and_operator(common::result& r);

        bool right_square_bracket(common::result& r);

        bool minus_equal_operator(common::result& r);

        bool divide_equal_operator(common::result& r);

        bool greater_than_operator(common::result& r);

        bool control_flow_operator(common::result& r);

        bool modulus_equal_operator(common::result& r);

        bool multiply_equal_operator(common::result& r);

        bool less_than_equal_operator(common::result& r);

        bool binary_or_equal_operator(common::result& r);

        bool binary_not_equal_operator(common::result& r);

        bool binary_and_equal_operator(common::result& r);

        bool greater_than_equal_operator(common::result& r);

    private:
        bool match_literal(
            common::result& r,
            const std::string& literal);

        common::rune_t read(
            common::result& r,
            bool skip_whitespace = true);

        bool read_dec_digits(
            common::result& r,
            size_t length,
            std::string& value);

        bool read_hex_digits(
            common::result& r,
            size_t length,
            std::string& value);

        std::string read_until(
            common::result& r,
            char target_ch);

        void rewind_one_char();

        void add_end_of_file_token();

        bool is_identifier(common::result& r);

        common::rune_t peek(common::result& r);

        std::string read_identifier(common::result& r);

        bool naked_identifier(common::result& r, bool add_token = true);

        bool type_tagged_identifier(common::result& r, bool add_token = true);

    private:
        static std::multimap<common::rune_t, lexer_case_callable> s_cases;

        bool _has_next = true;
        size_t _token_index = 0;
        uint32_t _paren_depth = 0;
        std::vector<token_t> _tokens {};
        common::source_file* _source_file = nullptr;
    };

}

