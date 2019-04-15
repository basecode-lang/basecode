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
#include "token_pool.h"

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

        // yield literal
        {'y', std::bind(&lexer::yield_literal, std::placeholders::_1, std::placeholders::_2)},

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

    token_t* lexer::next() {
        if (_tokens.empty())
            return nullptr;

        auto token = _tokens[_token_index];
        if (_token_index < _tokens.size()) {
            ++_token_index;
            return token;
        }

        return nullptr;
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

                    auto new_token = _tokens.back();
                    new_token->location.start(start_line->line, start_column);
                    new_token->location.end(end_line->line, end_column);
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
        auto token = token_pool::instance()->add(token_type_t::end_of_file);
        auto column = _source_file->column_by_index(_source_file->pos());
        auto line = _source_file->line_by_index(_source_file->pos());
        token->location.start(line->line, column);
        token->location.end(line->line, column);
        _tokens.emplace_back(token);
    }

    bool lexer::enum_literal(common::result& r) {
        auto value = match_literal(r, "enum"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::enum_literal,
                    value));
                return true;
            }
        }
        return false;
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

    bool lexer::break_literal(common::result& r) {
        auto value = match_literal(r, "break"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::break_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::while_literal(common::result& r) {
        auto value = match_literal(r, "while"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::while_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::struct_literal(common::result& r) {
        auto value = match_literal(r, "struct"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::struct_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::yield_literal(common::result& r) {
        auto value = match_literal(r, "yield"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::yield_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::union_literal(common::result& r) {
        auto value = match_literal(r, "union"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::union_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::continue_literal(common::result& r) {
        auto value = match_literal(r, "continue"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::continue_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::left_curly_brace(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == '{') {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::left_curly_brace,
                _source_file->make_slice(start_pos, 1)));
            return true;
        }
        return false;
    }

    bool lexer::right_curly_brace(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == '}') {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::right_curly_brace,
                _source_file->make_slice(start_pos, 1)));
            return true;
        }
        return false;
    }

    bool lexer::plus(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == '+') {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::plus,
                _source_file->make_slice(start_pos, 1)));
            return true;
        }
        return false;
    }

    bool lexer::bang(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == '!') {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::bang,
                _source_file->make_slice(start_pos, 1)));
            return true;
        }
        return false;
    }

    bool lexer::caret(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == '^') {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::caret,
                _source_file->make_slice(start_pos, 1)));
            return true;
        }
        return false;
    }

    bool lexer::tilde(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == '~') {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::tilde,
                _source_file->make_slice(start_pos, 1)));
            return true;
        }
        return false;
    }

    bool lexer::colon(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == ':') {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::colon,
                _source_file->make_slice(start_pos, 1)));
            return true;
        }
        return false;
    }

    bool lexer::minus(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == '-') {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::minus,
                _source_file->make_slice(start_pos, 1)));
            return true;
        }
        return false;
    }

    bool lexer::comma(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == ',') {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::comma,
                _source_file->make_slice(start_pos, 1)));
            return true;
        }
        return false;
    }

    bool lexer::slash(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == '/') {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::slash,
                _source_file->make_slice(start_pos, 1)));
            return true;
        }
        return false;
    }

    bool lexer::label(common::result& r) {
        auto ch = read(r);
        if (ch == '\'') {
            auto identifier = read_identifier(r);
            if (identifier.empty())
                return false;
            rewind_one_char();
            ch = read(r, false);
            if (ch == ':') {
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::label,
                    identifier));
                return true;
            }
        }
        return false;
    }

    bool lexer::period(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == '.') {
            ch = read(r, false);
            if (ch != '.') {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::period,
                    _source_file->make_slice(start_pos, 1)));
                return true;
            }
        }
        return false;
    }

    bool lexer::spread(common::result& r) {
        auto value = match_literal(r, "..."sv);
        if (!value.empty()) {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::spread_operator,
                value));
            return true;
        }
        return false;
    }

    bool lexer::percent(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == '%') {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::percent,
                _source_file->make_slice(start_pos, 1)));
            return true;
        }
        return false;
    }

    bool lexer::question(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == '?') {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::question,
                _source_file->make_slice(start_pos, 1)));
            return true;
        }
        return false;
    }

    bool lexer::asterisk(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == '*') {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::asterisk,
                _source_file->make_slice(start_pos, 1)));
            return true;
        }
        return false;
    }

    bool lexer::exponent(common::result& r) {
        auto value = match_literal(r, "**"sv);
        if (!value.empty()) {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::exponent,
                value));
            return true;
        }
        return false;
    }

    bool lexer::raw_block(common::result& r) {
        auto value = match_literal(r, "{{"sv);
        if (!value.empty()) {
            auto block_count = 1;

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
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::raw_block,
                _source_file->make_slice(start_pos, end_pos - start_pos)));
            return true;
        }
        return false;
    }

    bool lexer::attribute(common::result& r) {
        auto ch = read(r);
        if (ch == '@') {
            auto value = read_identifier(r);
            rewind_one_char();
            if (!value.empty()) {
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::attribute,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::directive(common::result& r) {
        auto ch = read(r);
        if (ch == '#') {
            auto value = read_identifier(r);
            if (!value.empty()) {
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::directive,
                    value));
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
        auto value = match_literal(r, ":="sv);
        if (!value.empty()) {
            if (_paren_depth == 0) {
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::assignment,
                    value));
            } else {
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::key_value_operator,
                    value));
            }
            return true;
        }
        return false;
    }

    bool lexer::left_paren(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == '(') {
            _paren_depth++;
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::left_paren,
                _source_file->make_slice(start_pos, 1)));
            return true;
        }
        return false;
    }

    bool lexer::in_literal(common::result& r) {
        auto value = match_literal(r, "in"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch) && ch != '_') {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::in_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::right_paren(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == ')') {
            if (_paren_depth > 0)
                _paren_depth--;
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::right_paren,
                _source_file->make_slice(start_pos, 1)));
            return true;
        }
        return false;
    }

    bool lexer::case_literal(common::result& r) {
        auto value = match_literal(r, "case"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (isspace(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::case_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::proc_literal(common::result& r) {
        auto value = match_literal(r, "proc"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::proc_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::ns_literal(common::result& r) {
        auto value = match_literal(r, "ns"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::namespace_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::if_literal(common::result& r) {
        auto value = match_literal(r, "if"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::if_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::xor_literal(common::result& r) {
        auto value = match_literal(r, "xor"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::xor_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::shl_literal(common::result& r) {
        auto value = match_literal(r, "shl"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::shl_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::shr_literal(common::result& r) {
        auto value = match_literal(r, "shr"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::shr_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::rol_literal(common::result& r) {
        auto value = match_literal(r, "rol"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::rol_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::ror_literal(common::result& r) {
        auto value = match_literal(r, "ror"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::ror_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::else_literal(common::result& r) {
        auto value = match_literal(r, "else"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::else_literal,
                    value));
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
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::line_comment,
                    read_until(r, '\n')));
                return true;
            }
        }
        return false;
    }

    bool lexer::for_literal(common::result& r) {
        auto value = match_literal(r, "for"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (isspace(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::for_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::nil_literal(common::result& r) {
        auto value = match_literal(r, "nil"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::nil_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::with_literal(common::result& r) {
        auto value = match_literal(r, "with"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::with_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::true_literal(common::result& r) {
        auto value = match_literal(r, "true"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::true_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::pipe_literal(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == '|') {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::pipe,
                _source_file->make_slice(start_pos, 1)));
            return true;
        }
        return false;
    }

    bool lexer::string_literal(common::result& r) {
        auto ch = read(r);
        if (ch == '\"') {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::string_literal,
                read_until(r, '"')));
            return true;
        }
        return false;
    }

    bool lexer::false_literal(common::result& r) {
        auto value = match_literal(r, "false"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::false_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::defer_literal(common::result& r) {
        auto value = match_literal(r, "defer"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::defer_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::number_literal(common::result& r) {
        uint8_t radix = 10;
        bool is_signed = false;
        std::string_view value {};
        number_types_t number_type = number_types_t::integer;

        auto ch = read(r);
        if (ch == '-') {
            ch = read(r, false);
            is_signed = true;
        }

        if (ch == '$') {
            radix = 16;

            auto start_pos = _source_file->pos();
            while (true) {
                ch = read(r, false);
                if (ch == '_')
                    continue;
                if (!isxdigit(ch))
                    break;
            }
            auto end_pos = _source_file->pos() - 1;
            value = _source_file->make_slice(start_pos, end_pos - start_pos);
        } else if (ch == '@') {
            radix = 8;

            const std::string valid = "012345678";
            auto start_pos = _source_file->pos();
            while (true) {
                ch = read(r, false);
                if (ch == '_')
                    continue;
                // XXX: requires utf8 fix
                if (valid.find_first_of(static_cast<char>(ch)) == std::string::npos)
                    break;
            }
            auto end_pos = _source_file->pos() - 1;
            value = _source_file->make_slice(start_pos, end_pos - start_pos);
        } else if (ch == '%') {
            radix = 2;

            auto start_pos = _source_file->pos();
            while (true) {
                ch = read(r, false);
                if (ch == '_')
                    continue;
                if (ch != '0' && ch != '1')
                    break;
            }
            auto end_pos = _source_file->pos() - 1;
            value = _source_file->make_slice(start_pos, end_pos - start_pos);
        } else {
            const std::string valid = "0123456789_.";
            auto start_pos = _source_file->pos() - 1;

            auto has_digits = false;

            // XXX: requires utf8 fix
            while (valid.find_first_of(static_cast<char>(ch)) != std::string::npos) {
                if (ch != '_') {
                    if (ch == '.') {
                        if (number_type != number_types_t::floating_point) {
                            number_type = number_types_t::floating_point;
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
            value = _source_file->make_slice(start_pos, end_pos - start_pos);
        }

        if (value.empty())
            return false;

        auto token = token_pool::instance()->add(token_type_t::number_literal, value);
        token->radix = radix;
        token->is_signed = is_signed;
        token->number_type = number_type;
        _tokens.emplace_back(token);

        rewind_one_char();

        return true;
    }

    bool lexer::switch_literal(common::result& r) {
        auto value = match_literal(r, "switch"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (isspace(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::switch_literal,
                    value));
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
                _tokens.emplace_back(token_pool::instance()->add(token_type_t::lambda_literal));
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
        auto value = match_literal(r, "import"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::import_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::scope_operator(common::result& r) {
        auto value = match_literal(r, "::"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (isalpha(ch) || ch == '_') {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::scope_operator,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::return_literal(common::result& r) {
        auto value = match_literal(r, "return"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::return_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::line_terminator(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == ';') {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::semi_colon,
                _source_file->make_slice(start_pos, 1)));
            return true;
        }
        return false;
    }

    bool lexer::equals_operator(common::result& r) {
        auto value = match_literal(r, "=="sv);
        if (!value.empty()) {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::equals,
                value));
            return true;
        }
        return false;
    }

    bool lexer::else_if_literal(common::result& r) {
        auto value = match_literal(r, "else if"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::else_if_literal,
                    value));
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
            if (ch == '_' || ch == '-' || isalnum(ch))
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
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::identifier,
                name));
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
                auto token = token_pool::instance()->add(
                    token_type_t::character_literal,
                    value);
                token->radix = radix;
                token->number_type = number_type;
                _tokens.emplace_back(token);
                return true;
            }
        }
        return false;
    }

    bool lexer::ampersand_literal(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == '&') {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::ampersand,
                _source_file->make_slice(start_pos, 1)));
            return true;
        }
        return false;
    }

    bool lexer::less_than_operator(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == '<') {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::less_than,
                _source_file->make_slice(start_pos, 1)));
            return true;
        }
        return false;
    }

    bool lexer::fallthrough_literal(common::result& r) {
        auto value = match_literal(r, "fallthrough"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::fallthrough_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::logical_or_operator(common::result& r) {
        auto value = match_literal(r, "||"sv);
        if (!value.empty()) {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::logical_or,
                value));
            return true;
        }
        return false;
    }

    bool lexer::block_comment(common::result& r) {
        auto value = match_literal(r, "/*"sv);
        if (!value.empty()) {
            auto block_count = 1;

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
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::block_comment,
                _source_file->make_slice(start_pos, end_pos - start_pos)));
            return true;
        }
        return false;
    }

    bool lexer::from_literal(common::result& r) {
        auto value = match_literal(r, "from"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::from_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::module_literal(common::result& r) {
        auto value = match_literal(r, "module"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (!isalnum(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::module_literal,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::value_sink_literal(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == '_') {
            ch = read(r, false);
            if (!isalnum(ch) && ch != '_') {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::value_sink_literal,
                    _source_file->make_slice(start_pos, 1)));
                return true;
            }
        }
        return false;
    }

    bool lexer::plus_equal_operator(common::result& r) {
        auto value = match_literal(r, "+:="sv);
        if (!value.empty()) {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::plus_equal_literal,
                value));
            return true;
        }
        return false;
    }

    bool lexer::constant_assignment(common::result& r) {
        auto value = match_literal(r, "::"sv);
        if (!value.empty()) {
            auto ch = read(r, false);
            if (isspace(ch)) {
                rewind_one_char();
                _tokens.emplace_back(token_pool::instance()->add(
                    token_type_t::constant_assignment,
                    value));
                return true;
            }
        }
        return false;
    }

    bool lexer::minus_equal_operator(common::result& r) {
        auto value = match_literal(r, "-:="sv);
        if (!value.empty()) {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::minus_equal_literal,
                value));
            return true;
        }
        return false;
    }

    bool lexer::logical_and_operator(common::result& r) {
        auto value = match_literal(r, "&&"sv);
        if (!value.empty()) {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::logical_and,
                value));
            return true;
        }
        return false;
    }

    bool lexer::not_equals_operator(common::result& r) {
        auto value = match_literal(r, "!="sv);
        if (!value.empty()) {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::not_equals,
                value));
            return true;
        }
        return false;
    }

    bool lexer::left_square_bracket(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == '[') {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::left_square_bracket,
                _source_file->make_slice(start_pos, 1)));
            return true;
        }
        return false;
    }

    bool lexer::right_square_bracket(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == ']') {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::right_square_bracket,
                _source_file->make_slice(start_pos, 1)));
            return true;
        }
        return false;
    }

    bool lexer::divide_equal_operator(common::result& r) {
        auto value = match_literal(r, "/:="sv);
        if (!value.empty()) {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::divide_equal_literal,
                value));
            return true;
        }
        return false;
    }

    bool lexer::greater_than_operator(common::result& r) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        if (ch == '>') {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::greater_than,
                _source_file->make_slice(start_pos, 1)));
            return true;
        }
        return false;
    }

    bool lexer::control_flow_operator(common::result& r) {
        auto value = match_literal(r, "=>"sv);
        if (!value.empty()) {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::control_flow_operator,
                value));
            return true;
        }
        return false;
    }

    bool lexer::modulus_equal_operator(common::result& r) {
        auto value = match_literal(r, "%:="sv);
        if (!value.empty()) {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::modulus_equal_literal,
                value));
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
                    _tokens.emplace_back(token_pool::instance()->add(
                        token_type_t::type_tagged_identifier,
                        name));
                }
                return true;
            }
        }

        return false;
    }

    bool lexer::multiply_equal_operator(common::result& r) {
        auto value = match_literal(r, "*:="sv);
        if (!value.empty()) {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::multiply_equal_literal,
                value));
            return true;
        }
        return false;
    }

    bool lexer::binary_or_equal_operator(common::result& r) {
        auto value = match_literal(r, "|:="sv);
        if (!value.empty()) {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::binary_or_equal_literal,
                value));
            return true;
        }
        return false;
    }

    bool lexer::less_than_equal_operator(common::result& r) {
        auto value = match_literal(r, "<="sv);
        if (!value.empty()) {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::less_than_equal,
                value));
            return true;
        }
        return false;
    }

    bool lexer::binary_not_equal_operator(common::result& r) {
        auto value = match_literal(r, "~:="sv);
        if (!value.empty()) {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::binary_not_equal_literal,
                value));
            return true;
        }
        return false;
    }

    bool lexer::binary_and_equal_operator(common::result& r) {
        auto value = match_literal(r, "&:="sv);
        if (!value.empty()) {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::binary_and_equal_literal,
                value));
            return true;
        }
        return false;
    }

    bool lexer::greater_than_equal_operator(common::result& r) {
        auto value = match_literal(r, ">="sv);
        if (!value.empty()) {
            _tokens.emplace_back(token_pool::instance()->add(
                token_type_t::greater_than_equal,
                value));
            return true;
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

    std::string_view lexer::match_literal(common::result& r, const std::string_view& literal) {
        auto start_pos = _source_file->pos();
        auto ch = read(r);
        for (const auto& target_ch : literal) {
            if (target_ch != ch)
                return {};
            ch = read(r, false);
        }
        rewind_one_char();
        return _source_file->make_slice(start_pos, _source_file->pos() - start_pos);
    }

}