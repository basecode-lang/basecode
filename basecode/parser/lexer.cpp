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

    common::rune_t lexer::peek() {
        while (!_source_file->eof()) {
            auto ch = _source_file->next(_result);
            if (_result.is_failed())
                return common::rune_invalid;
            if (!isspace(ch))
                return ch;
        }
        return 0;
    }

    bool lexer::has_next() const {
        return _has_next;
    }

    void lexer::rewind_one_char() {
        auto pos = _source_file->pos();
        if (pos == 0)
            return;
        _source_file->seek(pos - 1);
    }

    void lexer::set_token_location(token_t& token) {
        auto column = _source_file->column_by_index(_source_file->pos());
        auto source_line = _source_file->line_by_index(_source_file->pos());
        token.location.end(source_line->line, column);
        token.location.start(source_line->line, column);
    }

    bool lexer::next(token_t& token) {
        auto rune = read();
        if (rune == common::rune_invalid) {
            token = s_invalid;
            set_token_location(token);
            return false;
        }

        rune = rune > 0x80 ? rune : tolower(rune);
        if (rune == common::rune_eof) {
            token = s_end_of_file;
            set_token_location(token);
            return true;
        }

        rewind_one_char();
        _source_file->push_mark();
        defer({
            _source_file->pop_mark();
            _has_next = rune != common::rune_eof
                && rune != common::rune_invalid;
        });

        auto case_range = s_cases.equal_range(rune);
        for (auto it = case_range.first; it != case_range.second; ++it) {
            token.radix = 10;
            token.number_type = number_types_t::none;
            auto start_column = _source_file->column_by_index(_source_file->pos());
            auto start_line = _source_file->line_by_index(_source_file->pos());
            if (it->second(this, token)) {
                auto end_column = _source_file->column_by_index(_source_file->pos());
                auto end_line = _source_file->line_by_index(_source_file->pos());
                token.location.start(start_line->line, start_column);
                token.location.end(end_line->line, end_column);
                return true;
            }
            _source_file->restore_top_mark();
        }

        token = s_invalid;
        token.value = static_cast<char>(rune);
        set_token_location(token);

        return true;
    }

    const common::result& lexer::result() const {
        return _result;
    }

    common::rune_t lexer::read(bool skip_whitespace) {
        while (true) {
            auto ch = _source_file->next(_result);
            if (_result.is_failed())
                return common::rune_invalid;

            if (skip_whitespace && isspace(ch))
                continue;

            return ch;
        }
    }

    std::string lexer::read_identifier() {
        auto ch = read(false);
        if (ch != '_' && !isalpha(ch)) {
            return "";
        }
        std::stringstream stream;
        // XXX: requires utf8 fix
        stream << static_cast<char>(ch);
        while (true) {
            ch = read(false);
            if (ch == ';') {
                return stream.str();
            }
            if (ch == '_' || isalnum(ch)) {
                // XXX: requires utf8 fix
                stream << static_cast<char>(ch);
                continue;
            }
            return stream.str();
        }
    }

    bool lexer::enum_literal(token_t& token) {
        if (match_literal("enum")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_enum_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::break_literal(token_t& token) {
        if (match_literal("break")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_break_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::while_literal(token_t& token) {
        if (match_literal("while")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_while_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::struct_literal(token_t& token) {
        if (match_literal("struct")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_struct_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::union_literal(token_t& token) {
        if (match_literal("union")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_union_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::continue_literal(token_t& token) {
        if (match_literal("continue")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_continue_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::left_curly_brace(token_t& token) {
        auto ch = read();
        if (ch == '{') {
            token = s_left_curly_brace_literal;
            return true;
        }
        return false;
    }

    bool lexer::right_curly_brace(token_t& token) {
        auto ch = read();
        if (ch == '}') {
            token = s_right_curly_brace_literal;
            return true;
        }
        return false;
    }

    std::string lexer::read_until(char target_ch) {
        std::stringstream stream;
        while (true) {
            auto ch = read(false);
            if (ch == target_ch || ch == -1)
                break;
            // XXX: requires utf8 fix
            stream << static_cast<char>(ch);
        }
        return stream.str();
    }

    bool lexer::plus(token_t& token) {
        auto ch = read();
        if (ch == '+') {
            token = s_plus_literal;
            return true;
        }
        return false;
    }

    bool lexer::bang(token_t& token) {
        auto ch = read();
        if (ch == '!') {
            token = s_bang_literal;
            return true;
        }
        return false;
    }

    bool lexer::caret(token_t& token) {
        auto ch = read();
        if (ch == '^') {
            token = s_caret_literal;
            return true;
        }
        return false;
    }

    bool lexer::tilde(token_t& token) {
        auto ch = read();
        if (ch == '~') {
            token = s_tilde_literal;
            return true;
        }
        return false;
    }

    bool lexer::colon(token_t& token) {
        auto ch = read();
        if (ch == ':') {
            token = s_colon_literal;
            return true;
        }
        return false;
    }

    bool lexer::minus(token_t& token) {
        auto ch = read();
        if (ch == '-') {
            token = s_minus_literal;
            return true;
        }
        return false;
    }

    bool lexer::comma(token_t& token) {
        auto ch = read();
        if (ch == ',') {
            token = s_comma_literal;
            return true;
        }
        return false;
    }

    bool lexer::slash(token_t& token) {
        auto ch = read();
        if (ch == '/') {
            token = s_slash_literal;
            return true;
        }
        return false;
    }

    bool lexer::label(token_t& token) {
        auto ch = read();
        if (ch == '\'') {
            auto identifier = read_identifier();
            if (identifier.empty()) {
                return false;
            }
            rewind_one_char();
            ch = read(false);
            if (ch == ':') {
                token.type = token_types_t::label;
                token.value = identifier;
                return true;
            }
        }
        return false;
    }

    bool lexer::period(token_t& token) {
        auto ch = read();
        if (ch == '.') {
            ch = read();
            if (ch != '.') {
                rewind_one_char();
                token = s_period_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::spread(token_t& token) {
        if (match_literal("...")) {
            token = s_spread_operator_literal;
            return true;
        }
        return false;
    }

    bool lexer::percent(token_t& token) {
        auto ch = read();
        if (ch == '%') {
            token = s_percent_literal;
            return true;
        }
        return false;
    }

    bool lexer::question(token_t& token) {
        auto ch = read();
        if (ch == '?') {
            token = s_question_literal;
            return true;
        }
        return false;
    }

    bool lexer::asterisk(token_t& token) {
        auto ch = read();
        if (ch == '*') {
            token = s_asterisk_literal;
            return true;
        }
        return false;
    }

    bool lexer::exponent(token_t& token) {
        if (match_literal("**")) {
            token = s_exponent_literal;
            return true;
        }
        return false;
    }

    bool lexer::raw_block(token_t& token) {
        if (match_literal("{{")) {
            auto block_count = 1;
            token = s_raw_block;

            std::stringstream stream;
            while (true) {
                auto ch = read(false);
                if (ch == common::rune_eof) {
                    token = s_end_of_file;
                    set_token_location(token);
                    return true;
                }

                if (ch == '{') {
                    ch = read(false);
                    if (ch == '{') {
                        block_count++;
                        continue;
                    } else {
                        rewind_one_char();
                        ch = read(false);
                    }
                } else if (ch == '}') {
                    ch = read(false);
                    if (ch == '}') {
                        block_count--;
                        if (block_count == 0)
                            break;
                        continue;
                    } else {
                        rewind_one_char();
                        ch = read(false);
                    }
                }
                // XXX: requires utf8 fix
                stream << static_cast<char>(ch);
            }

            token.value = stream.str();
            return true;
        }
        return false;
    }

    bool lexer::attribute(token_t& token) {
        auto ch = read();
        if (ch == '@') {
            token.value = read_identifier();
            rewind_one_char();
            if (token.value.empty())
                return false;
            token.type = token_types_t::attribute;
            return true;
        }
        return false;
    }

    bool lexer::directive(token_t& token) {
        auto ch = read();
        if (ch == '#') {
            token.value = read_identifier();
            if (token.value.empty())
                return false;
            token.type = token_types_t::directive;
            return true;
        }
        return false;
    }

    bool lexer::identifier(token_t& token) {
        _source_file->push_mark();
        defer(_source_file->pop_mark());
        if (type_tagged_identifier(token))
            return true;
        _source_file->restore_top_mark();
        return naked_identifier(token);
    }

    bool lexer::assignment(token_t& token) {
        if (match_literal(":=")) {
            token = _paren_depth == 0 ?
                s_assignment_literal :
                s_key_value_operator;
            return true;
        }
        return false;
    }

    bool lexer::left_paren(token_t& token) {
        auto ch = read();
        if (ch == '(') {
            _paren_depth++;
            token = s_left_paren_literal;
            return true;
        }
        return false;
    }

    bool lexer::in_literal(token_t& token) {
        if (match_literal("in")) {
            auto ch = read(false);
            if (!isalnum(ch) && ch != '_') {
                rewind_one_char();
                token = s_in_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::right_paren(token_t& token) {
        auto ch = read();
        if (ch == ')') {
            if (_paren_depth > 0)
                _paren_depth--;
            token = s_right_paren_literal;
            return true;
        }
        return false;
    }

    bool lexer::case_literal(token_t& token) {
        if (match_literal("case")) {
            auto ch = read(false);
            if (isspace(ch)) {
                rewind_one_char();
                token = s_case_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::proc_literal(token_t& token) {
        if (match_literal("proc")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_proc_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::ns_literal(token_t& token) {
        if (match_literal("ns")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_namespace_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::if_literal(token_t& token) {
        if (match_literal("if")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_if_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::xor_literal(token_t& token) {
        if (match_literal("xor")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_xor_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::shl_literal(token_t& token) {
        if (match_literal("shl")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_shl_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::shr_literal(token_t& token) {
        if (match_literal("shr")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_shr_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::rol_literal(token_t& token) {
        if (match_literal("rol")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_rol_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::ror_literal(token_t& token) {
        if (match_literal("ror")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_ror_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::else_literal(token_t& token) {
        if (match_literal("else")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_else_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::line_comment(token_t& token) {
        auto ch = read();
        if (ch == '/') {
            ch = read(false);
            if (ch == '/') {
                token.type = token_types_t::line_comment;
                token.value = read_until('\n');
                //rewind_one_char();
                return true;
            }
        }
        return false;
    }

    bool lexer::for_literal(token_t& token) {
        if (match_literal("for")) {
            auto ch = read(false);
            if (isspace(ch)) {
                rewind_one_char();
                token = s_for_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::nil_literal(token_t& token) {
        if (match_literal("nil")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_nil_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::with_literal(token_t& token) {
        if (match_literal("with")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_with_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::true_literal(token_t& token) {
        if (match_literal("true")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_true_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::pipe_literal(token_t& token) {
        auto ch = read();
        if (ch == '|') {
            token = s_pipe_literal;
            return true;
        }
        return false;
    }

    bool lexer::string_literal(token_t& token) {
        auto ch = read();
        if (ch == '\"') {
            token.type = token_types_t::string_literal;
            token.value = read_until('"');
            return true;
        }
        return false;
    }

    bool lexer::false_literal(token_t& token) {
        if (match_literal("false")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_false_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::defer_literal(token_t& token) {
        if (match_literal("defer")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_defer_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::number_literal(token_t& token) {
        std::stringstream stream;
        token.type = token_types_t::number_literal;
        token.number_type = number_types_t::integer;

        auto ch = read();
        if (ch == '$') {
            token.radix = 16;
            while (true) {
                ch = read(false);
                if (ch == '_')
                    continue;
                if (!isxdigit(ch))
                    break;
                // XXX: requires utf8 fix
                stream << static_cast<char>(ch);
            }
        } else if (ch == '@') {
            const std::string valid = "012345678";
            token.radix = 8;
            while (true) {
                ch = read(false);
                if (ch == '_')
                    continue;
                // XXX: requires utf8 fix
                if (valid.find_first_of(static_cast<char>(ch)) == std::string::npos)
                    break;
                // XXX: requires utf8 fix
                stream << static_cast<char>(ch);
            }
        } else if (ch == '%') {
            token.radix = 2;
            while (true) {
                ch = read(false);
                if (ch == '_')
                    continue;
                if (ch != '0' && ch != '1')
                    break;
                // XXX: requires utf8 fix
                stream << static_cast<char>(ch);
            }
        } else {
            const std::string valid = "0123456789_.";

            if (ch == '-') {
                stream << '-';
                ch = read(false);
            }

            auto has_digits = false;

            // XXX: requires utf8 fix
            while (valid.find_first_of(static_cast<char>(ch)) != std::string::npos) {
                if (ch != '_') {
                    if (ch == '.') {
                        if (token.number_type != number_types_t::floating_point) {
                            token.number_type = number_types_t::floating_point;
                        } else {
                            token.type = token_types_t::invalid;
                            token.number_type = number_types_t::none;
                            return false;
                        }
                    }
                    // XXX: requires utf8 fix
                    stream << static_cast<char>(ch);
                    has_digits = true;
                }
                ch = read(false);
            }

            if (!has_digits)
                return false;
        }

        token.value = stream.str();
        if (token.value.empty())
            return false;

        rewind_one_char();

        return true;
    }

    bool lexer::switch_literal(token_t& token) {
        if (match_literal("switch")) {
            auto ch = read(false);
            if (isspace(ch)) {
                rewind_one_char();
                token = s_switch_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::lambda_literal(token_t& token) {
        auto ch = read();
        if (ch != '|')
            return false;

        _source_file->push_mark();
        defer({
            _source_file->restore_top_mark();
            _source_file->pop_mark();
        });

        token_t temp;
        while (true) {
            ch = read();
            if (ch == '|') {
                token.type = token_types_t::lambda_literal;
                return true;
            }

            if (ch == ',')
                continue;

            rewind_one_char();
            if (!identifier(temp))
                break;

            ch = read();
            if (ch == ':') {
                while (true) {
                    ch = read();
                    if (ch == '^' || ch == '[' || ch == ']')
                        continue;
                    break;
                }
                rewind_one_char();
                if (!identifier(temp))
                    return false;
            }
        }

        return false;
    }

    bool lexer::import_literal(token_t& token) {
        if (match_literal("import")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_import_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::scope_operator(token_t& token) {
        if (match_literal("::")) {
            auto ch = read(false);
            if (isalpha(ch) || ch == '_') {
                rewind_one_char();
                token.type = token_types_t::scope_operator;
                token.value = "::";
                return true;
            }
        }
        return false;
    }

    bool lexer::return_literal(token_t& token) {
        if (match_literal("return")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_return_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::line_terminator(token_t& token) {
        auto ch = read();
        if (ch == ';') {
            token = s_semi_colon_literal;
            return true;
        }
        return false;
    }

    bool lexer::equals_operator(token_t& token) {
        auto ch = read();
        if (ch == '=') {
            ch = read();
            if (ch == '=') {
                token = s_equals_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::else_if_literal(token_t& token) {
        if (match_literal("else if")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_else_if_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::naked_identifier(token_t& token) {
        auto name = read_identifier();

        if (name.empty())
            return false;

        rewind_one_char();

        token.value = name;
        token.type = token_types_t::identifier;

        return true;
    }

    bool lexer::character_literal(token_t& token) {
        uint8_t radix = 10;
        auto number_type = number_types_t::none;

        auto ch = read();
        if (ch == '\'') {
            std::string value {};
            ch = read(false);
            if (ch == '\\') {
                ch = read(false);
                switch (ch) {
                    case 'a': {
                        value = (char)0x07;
                        break;
                    }
                    case 'b': {
                        value = (char)0x08;
                        break;
                    }
                    case 'e': {
                        value = (char)0x1b;
                        break;
                    }
                    case 'n': {
                        value = (char)0x0a;
                        break;
                    }
                    case 'r': {
                        value = (char)0x0d;
                        break;
                    }
                    case 't': {
                        value = (char)0x09;
                        break;
                    }
                    case 'v': {
                        value = (char)0x0b;
                        break;
                    }
                    case '\\': {
                        value = "\\";
                        break;
                    }
                    case '\'': {
                        value = "'";
                        break;
                    }
                    case 'x': {
                        if (!read_hex_digits(2, value))
                            return false;
                        radix = 16;
                        number_type = number_types_t::integer;
                        break;
                    }
                    case 'u': {
                        if (!read_hex_digits(4, value))
                            return false;
                        radix = 16;
                        number_type = number_types_t::integer;
                        break;
                    }
                    case 'U': {
                        if (!read_hex_digits(8, value))
                            return false;
                        radix = 16;
                        number_type = number_types_t::integer;
                        break;
                    }
                    default: {
                        rewind_one_char();
                        if (!read_dec_digits(3, value))
                            return false;
                        radix = 8;
                        number_type = number_types_t::integer;
                    }
                }
            } else {
                value = static_cast<char>(ch);
            }
            ch = read();
            if (ch == '\'') {
                token.value = value;
                token.radix = radix;
                token.number_type = number_type;
                token.type = token_types_t::character_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::ampersand_literal(token_t& token) {
        auto ch = read();
        if (ch == '&') {
            token = s_ampersand_literal;
            return true;
        }
        return false;
    }

    bool lexer::less_than_operator(token_t& token) {
        auto ch = read();
        if (ch == '<') {
            token = s_less_than_literal;
            return true;
        }
        return false;
    }

    bool lexer::fallthrough_literal(token_t& token) {
        if (match_literal("fallthrough")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_fallthrough_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::logical_or_operator(token_t& token) {
        auto ch = read();
        if (ch == '|') {
            ch = read();
            if (ch == '|') {
                token = s_logical_or_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::block_comment(token_t& token) {
        if (match_literal("/*")) {
            auto block_count = 1;
            token = s_block_comment;

            std::stringstream stream;
            while (true) {
                auto ch = read(false);
                if (ch == common::rune_eof) {
                    token = s_end_of_file;
                    set_token_location(token);
                    return true;
                }

                if (ch == '/') {
                    ch = read(false);
                    if (ch == '*') {
                        block_count++;
                        continue;
                    } else {
                        rewind_one_char();
                        ch = read(false);
                    }
                } else if (ch == '*') {
                    ch = read(false);
                    if (ch == '/') {
                        block_count--;
                        if (block_count == 0)
                            break;
                        continue;
                    } else {
                        rewind_one_char();
                        ch = read(false);
                    }
                }
                // XXX: requires utf8 fix
                stream << static_cast<char>(ch);
            }

            token.value = stream.str();
            return true;
        }
        return false;
    }

    bool lexer::from_literal(token_t& token) {
        if (match_literal("from")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_from_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::module_literal(token_t& token) {
        if (match_literal("module")) {
            auto ch = read(false);
            if (!isalnum(ch)) {
                rewind_one_char();
                token = s_module_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::plus_equal_operator(token_t& token) {
        if (match_literal("+:=")) {
            token = s_plus_equal_literal;
            return true;
        }
        return false;
    }

    bool lexer::constant_assignment(token_t& token) {
        if (match_literal("::")) {
            auto ch = read(false);
            if (isspace(ch)) {
                rewind_one_char();
                token = s_constant_assignment_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::minus_equal_operator(token_t& token) {
        if (match_literal("-:=")) {
            token = s_minus_equal_literal;
            return true;
        }
        return false;
    }

    bool lexer::logical_and_operator(token_t& token) {
        auto ch = read();
        if (ch == '&') {
            ch = read();
            if (ch == '&') {
                token = s_logical_and_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::not_equals_operator(token_t& token) {
        auto ch = read();
        if (ch == '!') {
            ch = read();
            if (ch == '=') {
                token = s_not_equals_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::left_square_bracket(token_t& token) {
        auto ch = read();
        if (ch == '[') {
            token = s_left_square_bracket_literal;
            return true;
        }
        return false;
    }

    bool lexer::right_square_bracket(token_t& token) {
        auto ch = read();
        if (ch == ']') {
            token = s_right_square_bracket_literal;
            return true;
        }
        return false;
    }

    bool lexer::divide_equal_operator(token_t& token) {
        if (match_literal("/:=")) {
            token = s_divide_equal_literal;
            return true;
        }
        return false;
    }

    bool lexer::greater_than_operator(token_t& token) {
        auto ch = read();
        if (ch == '>') {
            token = s_greater_than_literal;
            return true;
        }
        return false;
    }

    bool lexer::control_flow_operator(token_t& token) {
        if (match_literal("=>")) {
            token = s_control_flow_operator;
            return true;
        }
        return false;
    }

    bool lexer::modulus_equal_operator(token_t& token) {
        if (match_literal("%:=")) {
            token = s_modulus_equal_literal;
            return true;
        }
        return false;
    }

    bool lexer::type_tagged_identifier(token_t& token) {
        auto name = read_identifier();

        if (name.empty())
            return false;

        rewind_one_char();

        _source_file->push_mark();
        defer({
            _source_file->restore_top_mark();
            _source_file->pop_mark();
        });

        auto ch = read(false);
        if (ch == '<') {
            auto is_tagged = false;

            token_t temp;
            while (true) {
                ch = read();
                // XXX: refactor this state machine so it handles
                //      classification better
                if (ch != '^')
                    rewind_one_char();
                if (!identifier(temp))
                    break;
                ch = read();
                if (ch == '<' || ch == '>') {
                    is_tagged = true;
                    break;
                } else if (ch != ',') {
                    break;
                }
                read();
                rewind_one_char();
            }

            if (is_tagged) {
                token.value = name;
                token.type = token_types_t::type_tagged_identifier;
                return true;
            }
        }

        return false;
    }

    bool lexer::multiply_equal_operator(token_t& token) {
        if (match_literal("*:=")) {
            token = s_multiply_equal_literal;
            return true;
        }
        return false;
    }

    bool lexer::binary_or_equal_operator(token_t& token) {
        if (match_literal("|:=")) {
            token = s_binary_or_equal_literal;
            return true;
        }
        return false;
    }

    bool lexer::less_than_equal_operator(token_t& token) {
        if (match_literal("<=")) {
            token = s_less_than_equal_literal;
            return true;
        }
        return false;
    }

    bool lexer::binary_not_equal_operator(token_t& token) {
        if (match_literal("~:=")) {
            token = s_binary_not_equal_literal;
            return true;
        }
        return false;
    }

    bool lexer::binary_and_equal_operator(token_t& token) {
        if (match_literal("&:=")) {
            token = s_binary_and_equal_literal;
            return true;
        }
        return false;
    }

    bool lexer::match_literal(const std::string& literal) {
        auto ch = read();
        for (const auto& target_ch : literal) {
            if (target_ch != ch)
                return false;
            ch = read(false);
        }
        rewind_one_char();
        return true;
    }

    bool lexer::greater_than_equal_operator(token_t& token) {
        auto ch = read();
        if (ch == '>') {
            ch = read();
            if (ch == '=') {
                token = s_greater_than_equal_literal;
                return true;
            }
        }
        return false;
    }

    bool lexer::read_hex_digits(size_t length, std::string& value) {
        while (length > 0) {
            auto ch = read(false);
            if (ch == '_')
                continue;
            if (isxdigit(ch)) {
                value += static_cast<char>(ch);
                --length;
            } else {
                return false;
            }
        }
        return true;
    }

    bool lexer::read_dec_digits(size_t length, std::string& value) {
        while (length > 0) {
            auto ch = read(false);
            if (ch == '_')
                continue;
            if (isdigit(ch)) {
                value += static_cast<char>(ch);
                --length;
            } else {
                return false;
            }
        }
        return true;
    }

};