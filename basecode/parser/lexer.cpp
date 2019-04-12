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

#include <sstream>
#include <fmt/format.h>
#include <common/defer.h>
#include "lexer.h"

namespace basecode::syntax {

    std::multimap<common::rune_t, lexer::lexer_case_callable> lexer::s_cases = {
        // attribute
        {'@', std::bind(&lexer::attribute, std::placeholders::_1, std::placeholders::_2)},

        // directive
        {'#', std::bind(&lexer::directive, std::placeholders::_1, std::placeholders::_2)},

        // +:=, add
        {'+', std::bind(&lexer::plus_equal_operator, std::placeholders::_1, std::placeholders::_2)},
        {'+', std::bind(&lexer::plus, std::placeholders::_1, std::placeholders::_2)},

        // /:=, block comment, line comment, slash
        {'/', std::bind(&lexer::divide_equal_operator, std::placeholders::_1, std::placeholders::_2)},
        {'/', std::bind(&lexer::block_comment, std::placeholders::_1, std::placeholders::_2)},
        {'/', std::bind(&lexer::line_comment, std::placeholders::_1, std::placeholders::_2)},
        {'/', std::bind(&lexer::slash, std::placeholders::_1, std::placeholders::_2)},

        // comma
        {',', std::bind(&lexer::comma, std::placeholders::_1, std::placeholders::_2)},

        // caret
        {'^', std::bind(&lexer::caret, std::placeholders::_1, std::placeholders::_2)},

        // not equals, bang
        //{0x2260, std::bind(&lexer::not_equals_operator, std::placeholders::_1, std::placeholders::_2)}, // ≠
        {'!',    std::bind(&lexer::not_equals_operator, std::placeholders::_1, std::placeholders::_2)},
        {'!',    std::bind(&lexer::bang, std::placeholders::_1, std::placeholders::_2)},

        // question
        {'?', std::bind(&lexer::question, std::placeholders::_1, std::placeholders::_2)},

        // period/spread
        {'.', std::bind(&lexer::period, std::placeholders::_1, std::placeholders::_2)},
        {'.', std::bind(&lexer::spread, std::placeholders::_1, std::placeholders::_2)},

        // ~:=, tilde
        {'~', std::bind(&lexer::binary_not_equal_operator, std::placeholders::_1, std::placeholders::_2)},
        {'~', std::bind(&lexer::tilde, std::placeholders::_1, std::placeholders::_2)},

        // assignment, scope operator, colon
        {':', std::bind(&lexer::constant_assignment, std::placeholders::_1, std::placeholders::_2)},
        {':', std::bind(&lexer::scope_operator, std::placeholders::_1, std::placeholders::_2)},
        {':', std::bind(&lexer::assignment, std::placeholders::_1, std::placeholders::_2)},
        {':', std::bind(&lexer::colon, std::placeholders::_1, std::placeholders::_2)},

        // %:=, percent, number literal
        {'%', std::bind(&lexer::modulus_equal_operator, std::placeholders::_1, std::placeholders::_2)},
        {'%', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'%', std::bind(&lexer::percent, std::placeholders::_1, std::placeholders::_2)},

        // *:=, exponent, asterisk
        {'*', std::bind(&lexer::multiply_equal_operator, std::placeholders::_1, std::placeholders::_2)},
        {'*', std::bind(&lexer::exponent, std::placeholders::_1, std::placeholders::_2)},
        {'*', std::bind(&lexer::asterisk, std::placeholders::_1, std::placeholders::_2)},

        // =>, equals
        {'=', std::bind(&lexer::control_flow_operator, std::placeholders::_1, std::placeholders::_2)},
        {'=', std::bind(&lexer::equals_operator, std::placeholders::_1, std::placeholders::_2)},

        // less than equal, less than
        {'<', std::bind(&lexer::less_than_equal_operator, std::placeholders::_1, std::placeholders::_2)},
        {'<', std::bind(&lexer::less_than_operator, std::placeholders::_1, std::placeholders::_2)},

        // greater than equal, greater than
        {'>', std::bind(&lexer::greater_than_equal_operator, std::placeholders::_1, std::placeholders::_2)},
        {'>', std::bind(&lexer::greater_than_operator, std::placeholders::_1, std::placeholders::_2)},

        // &:=, logical and, bitwise and, ampersand
        {'&', std::bind(&lexer::binary_and_equal_operator, std::placeholders::_1, std::placeholders::_2)},
        {'&', std::bind(&lexer::logical_and_operator, std::placeholders::_1, std::placeholders::_2)},
        {'&', std::bind(&lexer::ampersand_literal, std::placeholders::_1, std::placeholders::_2)},

        // lambda literal, |:=, logical or, bitwise or, pipe
        {'|', std::bind(&lexer::binary_or_equal_operator, std::placeholders::_1, std::placeholders::_2)},
        {'|', std::bind(&lexer::logical_or_operator, std::placeholders::_1, std::placeholders::_2)},
        {'|', std::bind(&lexer::lambda_literal, std::placeholders::_1, std::placeholders::_2)},
        {'|', std::bind(&lexer::pipe_literal, std::placeholders::_1, std::placeholders::_2)},

        // raw block/braces
        {'{', std::bind(&lexer::raw_block, std::placeholders::_1, std::placeholders::_2)},
        {'{', std::bind(&lexer::left_curly_brace, std::placeholders::_1, std::placeholders::_2)},
        {'}', std::bind(&lexer::right_curly_brace, std::placeholders::_1, std::placeholders::_2)},

        // parens
        {'(', std::bind(&lexer::left_paren, std::placeholders::_1, std::placeholders::_2)},
        {')', std::bind(&lexer::right_paren, std::placeholders::_1, std::placeholders::_2)},

        // square brackets
        {'[', std::bind(&lexer::left_square_bracket, std::placeholders::_1, std::placeholders::_2)},
        {']', std::bind(&lexer::right_square_bracket, std::placeholders::_1, std::placeholders::_2)},

        // line terminator
        {';', std::bind(&lexer::line_terminator, std::placeholders::_1, std::placeholders::_2)},

        // label/character literal
        {'\'', std::bind(&lexer::label, std::placeholders::_1, std::placeholders::_2)},
        {'\'', std::bind(&lexer::character_literal, std::placeholders::_1, std::placeholders::_2)},

        // string literal
        {'"', std::bind(&lexer::string_literal, std::placeholders::_1, std::placeholders::_2)},

        // return literal
        {'r', std::bind(&lexer::return_literal, std::placeholders::_1, std::placeholders::_2)},

        // true/fallthrough/false literals
        {'t', std::bind(&lexer::true_literal, std::placeholders::_1, std::placeholders::_2)},
        {'f', std::bind(&lexer::fallthrough_literal, std::placeholders::_1, std::placeholders::_2)},
        {'f', std::bind(&lexer::false_literal, std::placeholders::_1, std::placeholders::_2)},

        // nil/ns literals
        {'n', std::bind(&lexer::nil_literal, std::placeholders::_1, std::placeholders::_2)},
        {'n', std::bind(&lexer::ns_literal, std::placeholders::_1, std::placeholders::_2)},

        // module literals
        {'m', std::bind(&lexer::module_literal, std::placeholders::_1, std::placeholders::_2)},

        // import literal
        // if literal
        // in literal
        {'i', std::bind(&lexer::import_literal, std::placeholders::_1, std::placeholders::_2)},
        {'i', std::bind(&lexer::if_literal, std::placeholders::_1, std::placeholders::_2)},
        {'i', std::bind(&lexer::in_literal, std::placeholders::_1, std::placeholders::_2)},

        // enum literal
        // else if/else literals
        {'e', std::bind(&lexer::else_if_literal, std::placeholders::_1, std::placeholders::_2)},
        {'e', std::bind(&lexer::enum_literal, std::placeholders::_1, std::placeholders::_2)},
        {'e', std::bind(&lexer::else_literal, std::placeholders::_1, std::placeholders::_2)},

        // from/for literal
        {'f', std::bind(&lexer::from_literal, std::placeholders::_1, std::placeholders::_2)},
        {'f', std::bind(&lexer::for_literal, std::placeholders::_1, std::placeholders::_2)},

        // break literal
        {'b', std::bind(&lexer::break_literal, std::placeholders::_1, std::placeholders::_2)},

        // defer literal
        {'d', std::bind(&lexer::defer_literal, std::placeholders::_1, std::placeholders::_2)},

        // continue/case literal
        {'c', std::bind(&lexer::continue_literal, std::placeholders::_1, std::placeholders::_2)},
        {'c', std::bind(&lexer::case_literal, std::placeholders::_1, std::placeholders::_2)},

        // proc literal
        {'p', std::bind(&lexer::proc_literal, std::placeholders::_1, std::placeholders::_2)},

        // union literal
        {'u', std::bind(&lexer::union_literal, std::placeholders::_1, std::placeholders::_2)},

        // rol/ror literal
        {'r', std::bind(&lexer::rol_literal, std::placeholders::_1, std::placeholders::_2)},
        {'r', std::bind(&lexer::ror_literal, std::placeholders::_1, std::placeholders::_2)},

        // switch/struct/shl/shr literal
        {'s', std::bind(&lexer::switch_literal, std::placeholders::_1, std::placeholders::_2)},
        {'s', std::bind(&lexer::struct_literal, std::placeholders::_1, std::placeholders::_2)},
        {'s', std::bind(&lexer::shl_literal, std::placeholders::_1, std::placeholders::_2)},
        {'s', std::bind(&lexer::shr_literal, std::placeholders::_1, std::placeholders::_2)},

        // while literal
        {'w', std::bind(&lexer::while_literal, std::placeholders::_1, std::placeholders::_2)},
        {'w', std::bind(&lexer::with_literal, std::placeholders::_1, std::placeholders::_2)},

        // xor literal
        {'x', std::bind(&lexer::xor_literal, std::placeholders::_1, std::placeholders::_2)},

        // value sink literal
        {'_', std::bind(&lexer::value_sink_literal, std::placeholders::_1, std::placeholders::_2)},

        // identifier
        {'_', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'a', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'b', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'c', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'d', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'e', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'f', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'g', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'h', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'i', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'j', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'k', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'l', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'m', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'n', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'o', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'p', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'q', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'r', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'s', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'t', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'u', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'v', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'w', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'x', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'y', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'z', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},

        // number literal
        {'-', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'_', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'$', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'%', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'@', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'0', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'1', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'2', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'3', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'4', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'5', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'6', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'7', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'8', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'9', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},

        // -:=, minus, negate
        {'-', std::bind(&lexer::minus_equal_operator, std::placeholders::_1, std::placeholders::_2)},
        {'-', std::bind(&lexer::minus, std::placeholders::_1, std::placeholders::_2)},
    };

    lexer::lexer(common::source_file* source_file) : _source_file(source_file) {
    }

    common::rune_t lexer::peek(common::result& r) {
        while (!_source_file->eof()) {
            auto ch = _source_file->next(r);
            if (ch == common::rune_invalid)
                return ch;
            if (!isspace(ch))
                return ch;
        }
        return 0;
    }

    bool lexer::has_next() const {
        return _token_index < _tokens.size();
    }

    void lexer::rewind_one_char() {
        auto pos = _source_file->pos();
        if (pos == 0)
            return;
        _source_file->seek(pos - 1);
    }

    bool lexer::next(token_t& token) {
        if (_tokens.empty())
            return false;

        token = _tokens[_token_index];
        if (_token_index < _tokens.size()) {
            ++_token_index;
            return true;
        }

        return false;
    }

    bool lexer::tokenize(common::result& r) {
        _tokens.clear();
        _token_index = 0;
        _source_file->seek(0);

        while (true) {
            auto rune = read(r);
            if (rune == common::rune_eof) {
                add_end_of_file_token();
                return true;
            } else if (rune == common::rune_invalid) {
                r.error("X000", "invalid UTF-8 codepoint.");
                return false;
            }

            rune = rune > 0x80 ? rune : tolower(rune);
            rewind_one_char();
            _source_file->push_mark();

            auto start_column = _source_file->column_by_index(_source_file->pos());
            auto start_line = _source_file->line_by_index(_source_file->pos());

            auto case_range = s_cases.equal_range(rune);
            for (auto it = case_range.first; it != case_range.second; ++it) {
                if (it->second(this, r)) {
                    auto end_column = _source_file->column_by_index(_source_file->pos());
                    auto end_line = _source_file->line_by_index(_source_file->pos());

                    auto& new_token = _tokens.back();
                    new_token.location.start(start_line->line, start_column);
                    new_token.location.end(end_line->line, end_column);
                    break;
                }
                _source_file->restore_top_mark();
            }

            _source_file->pop_mark();
        }
    }

    common::rune_t lexer::read(
            common::result& r,
            bool skip_whitespace) {
        while (true) {
            auto ch = _source_file->next(r);
            if (ch == common::rune_invalid)
                return ch;

            if (skip_whitespace && isspace(ch))
                continue;

            return ch;
        }
    }

    void lexer::add_end_of_file_token() {
        auto token = s_end_of_file;
        auto column = _source_file->column_by_index(_source_file->pos());
        auto line = _source_file->line_by_index(_source_file->pos());
        token.location.start(line->line, column);
        token.location.end(line->line, column);
        _tokens.emplace_back(token);
    }

    bool lexer::enum_literal(common::result& r) {
        if (match_literal(r, "enum")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_enum_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::break_literal(common::result& r) {
        if (match_literal(r, "break")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_break_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::while_literal(common::result& r) {
        if (match_literal(r, "while")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_while_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::struct_literal(common::result& r) {
        if (match_literal(r, "struct")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_struct_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::yield_literal(common::result& r) {
        if (match_literal(r, "yield")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_yield_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::union_literal(common::result& r) {
        if (match_literal(r, "union")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_union_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::continue_literal(common::result& r) {
        if (match_literal(r, "continue")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_continue_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::left_curly_brace(common::result& r) {
        auto ch = read(r);
        if (ch == '{') {
            _tokens.emplace_back(s_left_curly_brace_literal);
            return true;
        }
        return false;
    }

    bool lexer::right_curly_brace(common::result& r) {
        auto ch = read(r);
        if (ch == '}') {
            _tokens.emplace_back(s_right_curly_brace_literal);
            return true;
        }
        return false;
    }

    bool lexer::plus(common::result& r) {
        auto ch = read(r);
        if (ch == '+') {
            _tokens.emplace_back(s_plus_literal);
            return true;
        }
        return false;
    }

    bool lexer::bang(common::result& r) {
        auto ch = read(r);
        if (ch == '!') {
            _tokens.emplace_back(s_bang_literal);
            return true;
        }
        return false;
    }

    bool lexer::caret(common::result& r) {
        auto ch = read(r);
        if (ch == '^') {
            _tokens.emplace_back(s_caret_literal);
            return true;
        }
        return false;
    }

    bool lexer::tilde(common::result& r) {
        auto ch = read(r);
        if (ch == '~') {
            _tokens.emplace_back(s_tilde_literal);
            return true;
        }
        return false;
    }

    bool lexer::colon(common::result& r) {
        auto ch = read(r);
        if (ch == ':') {
            _tokens.emplace_back(s_colon_literal);
            return true;
        }
        return false;
    }

    bool lexer::minus(common::result& r) {
        auto ch = read(r);
        if (ch == '-') {
            _tokens.emplace_back(s_minus_literal);
            return true;
        }
        return false;
    }

    bool lexer::comma(common::result& r) {
        auto ch = read(r);
        if (ch == ',') {
            _tokens.emplace_back(s_comma_literal);
            return true;
        }
        return false;
    }

    bool lexer::slash(common::result& r) {
        auto ch = read(r);
        if (ch == '/') {
            _tokens.emplace_back(s_slash_literal);
            return true;
        }
        return false;
    }

    bool lexer::label(common::result& r) {
        auto ch = read(r);
        if (ch == '\'') {
            auto identifier = read_identifier(r);
            if (identifier.empty()) {
                return false;
            }
            rewind_one_char();
            ch = read(r, false);
            if (ch == ':') {
                token_t token {};
                token.type = token_type_t::label;
                token.value = identifier;
                _tokens.emplace_back(token);
                return true;
            }
        }
        return false;
    }

    bool lexer::period(common::result& r) {
        auto ch = read(r);
        if (ch == '.') {
            ch = read(r, false);
            if (ch != '.') {
                rewind_one_char();
                _tokens.emplace_back(s_period_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::spread(common::result& r) {
        if (match_literal(r, "...")) {
            _tokens.emplace_back(s_spread_operator_literal);
            return true;
        }
        return false;
    }

    bool lexer::percent(common::result& r) {
        auto ch = read(r);
        if (ch == '%') {
            _tokens.emplace_back(s_percent_literal);
            return true;
        }
        return false;
    }

    bool lexer::question(common::result& r) {
        auto ch = read(r);
        if (ch == '?') {
            _tokens.emplace_back(s_question_literal);
            return true;
        }
        return false;
    }

    bool lexer::asterisk(common::result& r) {
        auto ch = read(r);
        if (ch == '*') {
            _tokens.emplace_back(s_asterisk_literal);
            return true;
        }
        return false;
    }

    bool lexer::exponent(common::result& r) {
        if (match_literal(r, "**")) {
            _tokens.emplace_back(s_exponent_literal);
            return true;
        }
        return false;
    }

    bool lexer::raw_block(common::result& r) {
        if (match_literal(r, "{{")) {
            auto block_count = 1;
            auto token = s_raw_block;

            auto start_pos = _source_file->pos();
            while (true) {
                auto ch = read(r, false);
                if (ch == common::rune_eof) {
                    add_end_of_file_token();
                    return true;
                }

                if (ch == '{') {
                    ch = read(r, false);
                    if (ch == '{') {
                        block_count++;
                        continue;
                    } else {
                        rewind_one_char();
                        read(r, false);
                    }
                } else if (ch == '}') {
                    ch = read(r, false);
                    if (ch == '}') {
                        block_count--;
                        if (block_count == 0)
                            break;
                        continue;
                    } else {
                        rewind_one_char();
                        read(r, false);
                    }
                }
            }

            auto end_pos = _source_file->pos();
            token.value = _source_file->make_slice(start_pos, end_pos - start_pos);
            _tokens.emplace_back(token);
            return true;
        }
        return false;
    }

    bool lexer::attribute(common::result& r) {
        auto ch = read(r);
        if (ch == '@') {
            token_t token {};
            token.type = token_type_t::attribute;
            token.value = read_identifier(r);
            rewind_one_char();
            if (!token.value.empty()) {
                _tokens.emplace_back(token);
                return true;
            }
        }
        return false;
    }

    bool lexer::directive(common::result& r) {
        auto ch = read(r);
        if (ch == '#') {
            token_t token {};
            token.type = token_type_t::directive;
            token.value = read_identifier(r);
            if (!token.value.empty()) {
                _tokens.emplace_back(token);
                return true;
            }
        }
        return false;
    }

    bool lexer::identifier(common::result& r) {
        _source_file->push_mark();
        defer(_source_file->pop_mark());
        if (type_tagged_identifier(r))
            return true;
        _source_file->restore_top_mark();
        return naked_identifier(r);
    }

    bool lexer::is_identifier(common::result& r) {
        _source_file->push_mark();
        defer(_source_file->pop_mark());
        if (type_tagged_identifier(r, false))
            return true;
        _source_file->restore_top_mark();
        return naked_identifier(r, false);
    }

    bool lexer::assignment(common::result& r) {
        if (match_literal(r, ":=")) {
            _tokens.emplace_back(_paren_depth == 0 ?
                s_assignment_literal :
                s_key_value_operator);
            return true;
        }
        return false;
    }

    bool lexer::left_paren(common::result& r) {
        auto ch = read(r);
        if (ch == '(') {
            _paren_depth++;
            _tokens.emplace_back(s_left_paren_literal);
            return true;
        }
        return false;
    }

    bool lexer::in_literal(common::result& r) {
        if (match_literal(r, "in")) {
            auto ch = read(r, false);
            if (!isalnum(ch) && ch != '_') {
                rewind_one_char();
                _tokens.emplace_back(s_in_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::right_paren(common::result& r) {
        auto ch = read(r);
        if (ch == ')') {
            if (_paren_depth > 0)
                _paren_depth--;
            _tokens.emplace_back(s_right_paren_literal);
            return true;
        }
        return false;
    }

    bool lexer::case_literal(common::result& r) {
        if (match_literal(r, "case")) {
            auto ch = read(r, false);
            if (isspace(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_case_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::proc_literal(common::result& r) {
        if (match_literal(r, "proc")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_proc_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::ns_literal(common::result& r) {
        if (match_literal(r, "ns")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_namespace_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::if_literal(common::result& r) {
        if (match_literal(r, "if")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_if_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::xor_literal(common::result& r) {
        if (match_literal(r, "xor")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_xor_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::shl_literal(common::result& r) {
        if (match_literal(r, "shl")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_shl_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::shr_literal(common::result& r) {
        if (match_literal(r, "shr")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_shr_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::rol_literal(common::result& r) {
        if (match_literal(r, "rol")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_rol_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::ror_literal(common::result& r) {
        if (match_literal(r, "ror")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_ror_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::else_literal(common::result& r) {
        if (match_literal(r, "else")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_else_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::line_comment(common::result& r) {
        auto ch = read(r);
        if (ch == '/') {
            ch = read(r, false);
            if (ch == '/') {
                token_t token{};
                token.type = token_type_t::line_comment;
                token.value = read_until(r, '\n');
                _tokens.emplace_back(token);
                return true;
            }
        }
        return false;
    }

    bool lexer::for_literal(common::result& r) {
        if (match_literal(r, "for")) {
            auto ch = read(r, false);
            if (isspace(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_for_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::nil_literal(common::result& r) {
        if (match_literal(r, "nil")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_nil_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::with_literal(common::result& r) {
        if (match_literal(r, "with")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_with_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::true_literal(common::result& r) {
        if (match_literal(r, "true")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_true_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::pipe_literal(common::result& r) {
        auto ch = read(r);
        if (ch == '|') {
            _tokens.emplace_back(s_pipe_literal);
            return true;
        }
        return false;
    }

    bool lexer::string_literal(common::result& r) {
        auto ch = read(r);
        if (ch == '\"') {
            token_t token{};
            token.type = token_type_t::string_literal;
            token.value = read_until(r, '"');
            _tokens.emplace_back(token);
            return true;
        }
        return false;
    }

    bool lexer::false_literal(common::result& r) {
        if (match_literal(r, "false")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_false_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::defer_literal(common::result& r) {
        if (match_literal(r, "defer")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_defer_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::number_literal(common::result& r) {
        token_t token{};
        token.radix = 10;
        token.type = token_type_t::number_literal;
        token.number_type = number_types_t::integer;

        auto ch = read(r);
        if (ch == '$') {
            auto start_pos = _source_file->pos();
            token.radix = 16;
            while (true) {
                ch = read(r, false);
                if (ch == '_')
                    continue;
                if (!isxdigit(ch))
                    break;
            }
            auto end_pos = _source_file->pos() - 1;
            token.value = _source_file->make_slice(start_pos, end_pos - start_pos);
        } else if (ch == '@') {
            const std::string valid = "012345678";
            auto start_pos = _source_file->pos();
            token.radix = 8;
            while (true) {
                ch = read(r, false);
                if (ch == '_')
                    continue;
                // XXX: requires utf8 fix
                if (valid.find_first_of(static_cast<char>(ch)) == std::string::npos)
                    break;
            }
            auto end_pos = _source_file->pos() - 1;
            token.value = _source_file->make_slice(start_pos, end_pos - start_pos);
        } else if (ch == '%') {
            token.radix = 2;
            auto start_pos = _source_file->pos();
            while (true) {
                ch = read(r, false);
                if (ch == '_')
                    continue;
                if (ch != '0' && ch != '1')
                    break;
            }
            auto end_pos = _source_file->pos() - 1;
            token.value = _source_file->make_slice(start_pos, end_pos - start_pos);
        } else {
            const std::string valid = "0123456789_.";
            auto start_pos = _source_file->pos() - 1;

            if (ch == '-')
                ch = read(r, false);

            auto has_digits = false;

            // XXX: requires utf8 fix
            while (valid.find_first_of(static_cast<char>(ch)) != std::string::npos) {
                if (ch != '_') {
                    if (ch == '.') {
                        if (token.number_type != number_types_t::floating_point) {
                            token.number_type = number_types_t::floating_point;
                        } else {
                            r.error("X000", "unexpected decimal in number.");
                            return false;
                        }
                    }
                    has_digits = true;
                }
                ch = read(r, false);
            }

            if (!has_digits)
                return false;

            auto end_pos = _source_file->pos() - 1;
            token.value = _source_file->make_slice(start_pos, end_pos - start_pos);
        }

        if (token.value.empty())
            return false;

        _tokens.emplace_back(token);
        rewind_one_char();

        return true;
    }

    bool lexer::switch_literal(common::result& r) {
        if (match_literal(r, "switch")) {
            auto ch = read(r, false);
            if (isspace(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_switch_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::lambda_literal(common::result& r) {
        auto ch = read(r);
        if (ch != '|')
            return false;

        _source_file->push_mark();
        defer({
            _source_file->restore_top_mark();
            _source_file->pop_mark();
        });

        while (true) {
            ch = read(r);
            if (ch == '|') {
                token_t token{};
                token.type = token_type_t::lambda_literal;
                _tokens.emplace_back(token);
                return true;
            }

            if (ch == ',')
                continue;

            rewind_one_char();
            if (!is_identifier(r))
                break;

            ch = read(r);
            if (ch == ':') {
                while (true) {
                    ch = read(r);
                    if (ch == '^' || ch == '[' || ch == ']')
                        continue;
                    break;
                }
                rewind_one_char();
                if (!is_identifier(r))
                    return false;
            }
        }

        return false;
    }

    bool lexer::import_literal(common::result& r) {
        if (match_literal(r, "import")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_import_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::scope_operator(common::result& r) {
        if (match_literal(r, "::")) {
            auto ch = read(r, false);
            if (isalpha(ch) || ch == '_') {
                rewind_one_char();
                _tokens.emplace_back(s_scope_operator_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::return_literal(common::result& r) {
        if (match_literal(r, "return")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_return_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::line_terminator(common::result& r) {
        auto ch = read(r);
        if (ch == ';') {
            _tokens.emplace_back(s_semi_colon_literal);
            return true;
        }
        return false;
    }

    bool lexer::equals_operator(common::result& r) {
        auto ch = read(r);
        if (ch == '=') {
            ch = read(r, false);
            if (ch == '=') {
                _tokens.emplace_back(s_equals_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::else_if_literal(common::result& r) {
        if (match_literal(r, "else if")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_else_if_literal);
                return true;
            }
        }
        return false;
    }

    std::string_view lexer::read_identifier(common::result& r) {
        auto start_pos = _source_file->pos();
        size_t end_pos = 0;

        auto ch = read(r, false);
        if (ch != '_' && !isalpha(ch)) {
            return ""sv;
        }

        while (true) {
            ch = read(r, false);
            if (ch == ';')
                break;
            if (ch == '_' || isalnum(ch))
                continue;
            break;
        }

        end_pos = _source_file->pos() - 1;
        return _source_file->make_slice(start_pos, end_pos - start_pos);
    }

    bool lexer::naked_identifier(common::result& r, bool add_token) {
        auto name = read_identifier(r);

        if (name.empty())
            return false;

        rewind_one_char();

        if (add_token) {
            token_t token{};
            token.value = name;
            token.type = token_type_t::identifier;
            _tokens.emplace_back(token);
        }

        return true;
    }

    bool lexer::character_literal(common::result& r) {
        uint8_t radix = 10;
        auto number_type = number_types_t::none;

        auto ch = read(r);
        if (ch == '\'') {
            std::string_view value {};
            ch = read(r, false);
            if (ch == '\\') {
                ch = read(r, false);
                switch (ch) {
                    case 'a': {
                        value = "\x07"sv;
                        break;
                    }
                    case 'b': {
                        value = "\x08"sv;
                        break;
                    }
                    case 'e': {
                        value = "\x1b"sv;
                        break;
                    }
                    case 'n': {
                        value = "\x0a"sv;
                        break;
                    }
                    case 'r': {
                        value = "\x0d"sv;
                        break;
                    }
                    case 't': {
                        value = "\x09"sv;
                        break;
                    }
                    case 'v': {
                        value = "\x0b"sv;
                        break;
                    }
                    case '\\': {
                        value = R"(\)"sv;
                        break;
                    }
                    case '\'': {
                        value = "'"sv;
                        break;
                    }
                    case 'x': {
                        if (!read_hex_digits(r, 2, value))
                            return false;
                        radix = 16;
                        number_type = number_types_t::integer;
                        break;
                    }
                    case 'u': {
                        if (!read_hex_digits(r, 4, value))
                            return false;
                        radix = 16;
                        number_type = number_types_t::integer;
                        break;
                    }
                    case 'U': {
                        if (!read_hex_digits(r, 8, value))
                            return false;
                        radix = 16;
                        number_type = number_types_t::integer;
                        break;
                    }
                    default: {
                        rewind_one_char();
                        if (!read_dec_digits(r, 3, value))
                            return false;
                        radix = 8;
                        number_type = number_types_t::integer;
                    }
                }
            } else {
                value = _source_file->make_slice(_source_file->pos() - 1, 1);
            }
            ch = read(r, false);
            if (ch == '\'') {
                token_t token{};
                token.value = value;
                token.radix = radix;
                token.number_type = number_type;
                token.type = token_type_t::character_literal;
                _tokens.emplace_back(token);
                return true;
            }
        }
        return false;
    }

    bool lexer::ampersand_literal(common::result& r) {
        auto ch = read(r);
        if (ch == '&') {
            _tokens.emplace_back(s_ampersand_literal);
            return true;
        }
        return false;
    }

    bool lexer::less_than_operator(common::result& r) {
        auto ch = read(r);
        if (ch == '<') {
            _tokens.emplace_back(s_less_than_literal);
            return true;
        }
        return false;
    }

    bool lexer::fallthrough_literal(common::result& r) {
        if (match_literal(r, "fallthrough")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_fallthrough_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::logical_or_operator(common::result& r) {
        auto ch = read(r);
        if (ch == '|') {
            ch = read(r, false);
            if (ch == '|') {
                _tokens.emplace_back(s_logical_or_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::block_comment(common::result& r) {
        if (match_literal(r, "/*")) {
            auto block_count = 1;
            auto token = s_block_comment;

            auto start_pos = _source_file->pos();
            while (true) {
                auto ch = read(r, false);
                if (ch == common::rune_eof) {
                    add_end_of_file_token();
                    return true;
                }

                if (ch == '/') {
                    ch = read(r, false);
                    if (ch == '*') {
                        block_count++;
                        continue;
                    } else {
                        rewind_one_char();
                        read(r, false);
                    }
                } else if (ch == '*') {
                    ch = read(r, false);
                    if (ch == '/') {
                        block_count--;
                        if (block_count == 0)
                            break;
                        continue;
                    } else {
                        rewind_one_char();
                        read(r, false);
                    }
                }
            }

            auto end_pos = _source_file->pos();
            token.value = _source_file->make_slice(start_pos, end_pos - start_pos);
            _tokens.emplace_back(token);
            return true;
        }
        return false;
    }

    bool lexer::from_literal(common::result& r) {
        if (match_literal(r, "from")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_from_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::module_literal(common::result& r) {
        if (match_literal(r, "module")) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_module_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::value_sink_literal(common::result& r) {
        auto ch = read(r);
        if (ch == '_') {
            ch = read(r, false);
            if (!isalnum(ch) && ch != '_') {
                rewind_one_char();
                _tokens.emplace_back(s_value_sink_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::plus_equal_operator(common::result& r) {
        if (match_literal(r, "+:=")) {
            _tokens.emplace_back(s_plus_equal_literal);
            return true;
        }
        return false;
    }

    bool lexer::constant_assignment(common::result& r) {
        if (match_literal(r, "::")) {
            auto ch = read(r, false);
            if (isspace(ch)) {
                rewind_one_char();
                _tokens.emplace_back(s_constant_assignment_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::minus_equal_operator(common::result& r) {
        if (match_literal(r, "-:=")) {
            _tokens.emplace_back(s_minus_equal_literal);
            return true;
        }
        return false;
    }

    bool lexer::logical_and_operator(common::result& r) {
        auto ch = read(r);
        if (ch == '&') {
            ch = read(r, false);
            if (ch == '&') {
                _tokens.emplace_back(s_logical_and_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::not_equals_operator(common::result& r) {
        auto ch = read(r);
        if (ch == '!') {
            ch = read(r, false);
            if (ch == '=') {
                _tokens.emplace_back(s_not_equals_literal);
                return true;
            }
        }
        return false;
    }

    bool lexer::left_square_bracket(common::result& r) {
        auto ch = read(r);
        if (ch == '[') {
            _tokens.emplace_back(s_left_square_bracket_literal);
            return true;
        }
        return false;
    }

    bool lexer::right_square_bracket(common::result& r) {
        auto ch = read(r);
        if (ch == ']') {
            _tokens.emplace_back(s_right_square_bracket_literal);
            return true;
        }
        return false;
    }

    bool lexer::divide_equal_operator(common::result& r) {
        if (match_literal(r, "/:=")) {
            _tokens.emplace_back(s_divide_equal_literal);
            return true;
        }
        return false;
    }

    bool lexer::greater_than_operator(common::result& r) {
        auto ch = read(r);
        if (ch == '>') {
            _tokens.emplace_back(s_greater_than_literal);
            return true;
        }
        return false;
    }

    bool lexer::control_flow_operator(common::result& r) {
        if (match_literal(r, "=>")) {
            _tokens.emplace_back(s_control_flow_operator);
            return true;
        }
        return false;
    }

    bool lexer::modulus_equal_operator(common::result& r) {
        if (match_literal(r, "%:=")) {
            _tokens.emplace_back(s_modulus_equal_literal);
            return true;
        }
        return false;
    }

    bool lexer::type_tagged_identifier(common::result& r, bool add_token) {
        auto name = read_identifier(r);

        if (name.empty())
            return false;

        rewind_one_char();

        _source_file->push_mark();
        defer({
            _source_file->restore_top_mark();
            _source_file->pop_mark();
        });

        auto ch = read(r, false);
        if (ch == '<') {
            auto is_tagged = false;

            while (true) {
                ch = read(r);
                // XXX: refactor this state machine so it handles
                //      classification better
                if (ch != '^')
                    rewind_one_char();
                if (!is_identifier(r))
                    break;
                ch = read(r);
                if (ch == '<' || ch == '>') {
                    is_tagged = true;
                    break;
                } else if (ch != ',') {
                    break;
                }
                read(r);
                rewind_one_char();
            }

            if (is_tagged) {
                if (add_token) {
                    token_t token{};
                    token.value = name;
                    token.type = token_type_t::type_tagged_identifier;
                    _tokens.emplace_back(token);
                }
                return true;
            }
        }

        return false;
    }

    bool lexer::multiply_equal_operator(common::result& r) {
        if (match_literal(r, "*:=")) {
            _tokens.emplace_back(s_multiply_equal_literal);
            return true;
        }
        return false;
    }

    bool lexer::binary_or_equal_operator(common::result& r) {
        if (match_literal(r, "|:=")) {
            _tokens.emplace_back(s_binary_or_equal_literal);
            return true;
        }
        return false;
    }

    bool lexer::less_than_equal_operator(common::result& r) {
        if (match_literal(r, "<=")) {
            _tokens.emplace_back(s_less_than_equal_literal);
            return true;
        }
        return false;
    }

    bool lexer::binary_not_equal_operator(common::result& r) {
        if (match_literal(r, "~:=")) {
            _tokens.emplace_back(s_binary_not_equal_literal);
            return true;
        }
        return false;
    }

    bool lexer::binary_and_equal_operator(common::result& r) {
        if (match_literal(r, "&:=")) {
            _tokens.emplace_back(s_binary_and_equal_literal);
            return true;
        }
        return false;
    }

    bool lexer::greater_than_equal_operator(common::result& r) {
        auto ch = read(r);
        if (ch == '>') {
            ch = read(r, false);
            if (ch == '=') {
                _tokens.emplace_back(s_greater_than_equal_literal);
                return true;
            }
        }
        return false;
    }

    std::string_view lexer::read_until(common::result& r, char target_ch) {
        auto start_pos = _source_file->pos();
        while (true) {
            auto ch = read(r, false);
            if (ch == target_ch || ch == -1)
                break;
        }
        auto end_pos = _source_file->pos() - 1;
        return _source_file->make_slice(start_pos, end_pos - start_pos);
    }

    bool lexer::match_literal(common::result& r, const std::string& literal) {
        auto ch = read(r);
        for (const auto& target_ch : literal) {
            if (target_ch != ch)
                return false;
            ch = read(r, false);
        }
        rewind_one_char();
        return true;
    }

    bool lexer::read_hex_digits(common::result& r, size_t length, std::string_view& value) {
        auto start_pos = _source_file->pos();
        while (length > 0) {
            auto ch = read(r, false);
            if (ch == '_')
                continue;
            if (isxdigit(ch)) {
                --length;
            } else {
                return false;
            }
        }
        value = _source_file->make_slice(start_pos, _source_file->pos() - start_pos);
        return true;
    }

    bool lexer::read_dec_digits(common::result& r, size_t length, std::string_view& value) {
        auto start_pos = _source_file->pos();
        while (length > 0) {
            auto ch = read(r, false);
            if (ch == '_')
                continue;
            if (isdigit(ch)) {
                --length;
            } else {
                return false;
            }
        }
        value = _source_file->make_slice(start_pos, _source_file->pos() - start_pos);
        return true;
    }

}