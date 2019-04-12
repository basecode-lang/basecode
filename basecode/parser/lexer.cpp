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
#include <boost/container/flat_map.hpp>
#include "lexer.h"

#define NEXT_CHARACTER { \
        _source_file->pop_mark(); \
        ch = read(r); \
        if (ch == common::rune_eof) { \
            _tokens.push_back(s_end_of_file); \
            return true; \
        } else if (ch == common::rune_invalid) { \
            r.error("X000", "cannot read invalid UTF-8 codepoint."); \
            return false; \
        } \
        rewind_one_char(); \
        pos = _source_file->pos(); \
        _source_file->push_mark(); \
        current_lexer = &char_lexers[pos]; \
        if (current_lexer->lexer_count == 0) { \
            r.error( \
                "X000", \
                fmt::format( \
                    "no lexer found: {} @ {}:{}", \
                    pos, \
                    start_line, \
                    start_column)); \
            return false; \
        } \
        start_column = current_lexer->start_column; \
        start_line = current_lexer->start_line; \
        token.location.start(start_line, start_column); \
        goto *(current_lexer->lexers[current_lexer->lexer_index++]); \
    }
#define MATCH(t) { \
        pos = _source_file->pos(); \
        auto end_column = _source_file->column_by_index(pos); \
        auto end_line = _source_file->line_by_index(pos); \
        t.location.end(end_line->line, end_column); \
        _tokens.push_back(t); \
        NEXT_CHARACTER \
    }
#define MISMATCH { \
        if (current_lexer->lexer_count == 0 \
        ||  current_lexer->lexer_index >= current_lexer->lexer_count) { \
            r.error( \
                "X000", \
                fmt::format( \
                    "no lexer found: {} @ {}:{}", \
                    pos, \
                    start_line, \
                    start_column)); \
            return false; \
        } \
        _source_file->restore_top_mark(); \
        goto *(current_lexer->lexers[current_lexer->lexer_index++]); \
    }

namespace basecode::syntax {

    lexer::lexer(common::source_file* source_file) : _source_file(source_file) {
    }

    bool lexer::identifier(
            common::result& r,
            token_t& token) {
        _source_file->push_mark();
        defer(_source_file->pop_mark());
        if (type_tagged_identifier(r, token))
            return true;
        _source_file->restore_top_mark();
        return naked_identifier(r, token);
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

    bool lexer::read_hex_digits(
            common::result& r,
            size_t length,
            std::string& value) {
        while (length > 0) {
            auto ch = read(r, false);
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

    bool lexer::read_dec_digits(
            common::result& r,
            size_t length,
            std::string& value) {
        while (length > 0) {
            auto ch = read(r, false);
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

    bool lexer::naked_identifier(
            common::result& r,
            token_t& token) {
        auto name = read_identifier(r);

        if (name.empty())
            return false;

        rewind_one_char();

        token.value = name;
        token.type = token_type_t::identifier;

        return true;
    }

    bool lexer::has_next() const {
        return !_tokens.empty();
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

        token = _tokens[_index];
        if (_index < _tokens.size()) {
            ++_index;
            return true;
        }
        return false;
    }

    bool lexer::type_tagged_identifier(
            common::result& r,
            token_t& token) {
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

            token_t temp;
            while (true) {
                ch = read(r);
                // XXX: refactor this state machine so it handles
                //      classification better
                if (ch != '^')
                    rewind_one_char();
                if (!identifier(r, temp))
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
                token.value = name;
                token.type = token_type_t::type_tagged_identifier;
                return true;
            }
        }

        return false;
    }

    bool lexer::tokenize(common::result& r) {
        auto start = std::chrono::high_resolution_clock::now();
        defer({
            auto end = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            fmt::print("{}, tokenize time = {}\n", _source_file->path().string(), elapsed.count());
        });

        const std::string valid_octal = "012345678";
        const std::string valid_decimal = "0123456789_.";

        const static boost::container::flat_multimap<common::rune_t, void*> s_cases = {
            // attribute
            {'@', &&attribute},

            // directive
            {'#', &&directive},

            // +:=, add
            {'+', &&plus_equal_operator},
            {'+', &&plus},

            // /:=, block comment, line comment, slash
            {'/', &&divide_equal_operator},
            {'/', &&block_comment},
            {'/', &&line_comment},
            {'/', &&slash},

            // comma
            {',', &&comma},

            // caret
            {'^', &&caret},

            // not equals, bang
            //{0x2260, &&not_equals_operator}, // ≠
            {'!', &&not_equals_operator},
            {'!', &&bang},

            // question
            {'?', &&question},

            // period/spread
            {'.', &&period},
            {'.', &&spread},

            // ~:=, tilde
            {'~', &&binary_not_equal_operator},
            {'~', &&tilde},

            // assignment, scope operator, colon
            {':', &&constant_assignment},
            {':', &&scope_operator},
            {':', &&assignment},
            {':', &&colon},

            // %:=, percent, number literal
            {'%', &&modulus_equal_operator},
            {'%', &&number_literal},
            {'%', &&percent},

            // *:=, exponent, asterisk
            {'*', &&multiply_equal_operator},
            {'*', &&exponent},
            {'*', &&asterisk},

            // =>, equals
            {'=', &&control_flow_operator},
            {'=', &&equals_operator},

            // less than equal, less than
            {'<', &&less_than_equal_operator},
            {'<', &&less_than_operator},

            // greater than equal, greater than
            {'>', &&greater_than_equal_operator},
            {'>', &&greater_than_operator},

            // &:=, logical and, bitwise and, ampersand
            {'&', &&binary_and_equal_operator},
            {'&', &&logical_and_operator},
            {'&', &&ampersand_literal},

            // lambda literal, |:=, logical or, bitwise or, pipe
            {'|', &&binary_or_equal_operator},
            {'|', &&logical_or_operator},
            {'|', &&lambda_literal},
            {'|', &&pipe_literal},

            // raw block/braces
            {'{', &&raw_block},
            {'{', &&left_curly_brace},
            {'}', &&right_curly_brace},

            // parens
            {'(', &&left_paren},
            {')', &&right_paren},

            // square brackets
            {'[', &&left_square_bracket},
            {']', &&right_square_bracket},

            // line terminator
            {';', &&line_terminator},

            // label/character literal
            {'\'', &&label},
            {'\'', &&character_literal},

            // string literal
            {'"', &&string_literal},

            // return literal
            {'r', &&return_literal},

            // true/fallthrough/false literals
            {'t', &&true_literal},
            {'f', &&fallthrough_literal},
            {'f', &&false_literal},

            // nil/ns literals
            {'n', &&nil_literal},
            {'n', &&ns_literal},

            // module literals
            {'m', &&module_literal},

            // import literal
            // if literal
            // in literal
            {'i', &&import_literal},
            {'i', &&if_literal},
            {'i', &&in_literal},

            // enum literal
            // else if/else literals
            {'e', &&else_if_literal},
            {'e', &&enum_literal},
            {'e', &&else_literal},

            // from/for literal
            {'f', &&from_literal},
            {'f', &&for_literal},

            // break literal
            {'b', &&break_literal},

            // defer literal
            {'d', &&defer_literal},

            // continue/case literal
            {'c', &&continue_literal},
            {'c', &&case_literal},

            // proc literal
            {'p', &&proc_literal},

            // union literal
            {'u', &&union_literal},

            // rol/ror literal
            {'r', &&rol_literal},
            {'r', &&ror_literal},

            // switch/struct/shl/shr literal
            {'s', &&switch_literal},
            {'s', &&struct_literal},
            {'s', &&shl_literal},
            {'s', &&shr_literal},

            // while literal
            {'w', &&while_literal},
            {'w', &&with_literal},

            // xor literal
            {'x', &&xor_literal},

            // yield literal
            {'y', &&yield_literal},

            // value sink literal
            {'_', &&value_sink_literal},

            // identifier
            {'_', &&identifier},
            {'a', &&identifier},
            {'b', &&identifier},
            {'c', &&identifier},
            {'d', &&identifier},
            {'e', &&identifier},
            {'f', &&identifier},
            {'g', &&identifier},
            {'h', &&identifier},
            {'i', &&identifier},
            {'j', &&identifier},
            {'k', &&identifier},
            {'l', &&identifier},
            {'m', &&identifier},
            {'n', &&identifier},
            {'o', &&identifier},
            {'p', &&identifier},
            {'q', &&identifier},
            {'r', &&identifier},
            {'s', &&identifier},
            {'t', &&identifier},
            {'u', &&identifier},
            {'v', &&identifier},
            {'w', &&identifier},
            {'x', &&identifier},
            {'y', &&identifier},
            {'z', &&identifier},

            // number literal
            {'-', &&number_literal},
            {'_', &&number_literal},
            {'$', &&number_literal},
            {'%', &&number_literal},
            {'@', &&number_literal},
            {'0', &&number_literal},
            {'1', &&number_literal},
            {'2', &&number_literal},
            {'3', &&number_literal},
            {'4', &&number_literal},
            {'5', &&number_literal},
            {'6', &&number_literal},
            {'7', &&number_literal},
            {'8', &&number_literal},
            {'9', &&number_literal},

            // -:=, minus, negate
            {'-', &&minus_equal_operator},
            {'-', &&minus},
        };

        struct character_lexers_t {
            void* lexers[16];
            size_t lexer_count = 0;
            size_t lexer_index = 0;
            uint32_t start_line = 0;
            uint32_t start_column = 0;
        };

        common::rune_t ch;
        std::vector<character_lexers_t> char_lexers {};
        char_lexers.resize(_source_file->length());

        while (true) {
            ch = read(r);
            if (ch == common::rune_eof) {
                break;
            } else if (ch == common::rune_invalid) {
                r.error("X000", "invalid unicode character in source file.");
                return false;
            }

            const auto pos = _source_file->pos() - 1;
            auto& char_lexer = char_lexers[pos];

            ch = ch > 0x80 ? ch : tolower(ch);
            auto case_range = s_cases.equal_range(ch);
            for (auto it = case_range.first; it != case_range.second; ++it) {
                char_lexer.lexers[char_lexer.lexer_count++] = it->second;
            }

            char_lexer.start_line = _source_file->line_by_index(pos)->line;
            char_lexer.start_column = _source_file->column_by_index(pos);
        }

        _source_file->seek(0);

        size_t pos = 0;
        token_t token{};
        std::string temp{};
        token_t temp_token{};
        std::stringstream stream{};
        character_lexers_t* current_lexer = nullptr;

        uint32_t start_line = 0;
        uint32_t start_column = 0;

        _tokens.reserve(4096);

        NEXT_CHARACTER

        return !r.is_failed();

        plus:
        {
            ch = read(r);
            if (ch == '+')
                MATCH(s_plus_literal)
            MISMATCH
        }

        bang:
        {
            ch = read(r);
            if (ch == '!')
                MATCH(s_bang_literal)
            MISMATCH
        }

        minus:
        {
            ch = read(r);
            if (ch == '-')
                MATCH(s_minus_literal)
            MISMATCH
        }

        comma:
        {
            ch = read(r);
            if (ch == ',')
                MATCH(s_comma_literal)
            MISMATCH
        }

        slash:
        {
            ch = read(r);
            if (ch == '/')
                MATCH(s_slash_literal)
            MISMATCH
        }

        caret:
        {
            ch = read(r);
            if (ch == '^')
                MATCH(s_caret_literal)
            MISMATCH
        }

        tilde:
        {
            ch = read(r);
            if (ch == '~')
                MATCH(s_tilde_literal)
            MISMATCH
        }

        colon:
        {
            ch = read(r);
            if (ch == ':')
                MATCH(s_colon_literal)
            MISMATCH
        }

        label:
        {
            ch = read(r);
            if (ch == '\'') {
                temp = read_identifier(r);
                if (!temp.empty()) {
                    rewind_one_char();
                    ch = read(r, false);
                    if (ch == ':') {
                        token.radix = 10;
                        token.type = token_type_t::label;
                        token.value = temp;
                        token.number_type = number_types_t::none;
                        MATCH(token)
                    }
                }
            }
            MISMATCH
        }

        period:
        {
            ch = read(r);
            if (ch == '.') {
                ch = read(r);
                if (ch != '.') {
                    rewind_one_char();
                    MATCH(s_period_literal)
                }
            }
            MISMATCH
        }

        spread:
        {
            if (match_literal(r, "..."))
                MATCH(s_spread_operator_literal)
            MISMATCH
        }

        percent:
        {
            ch = read(r);
            if (ch == '%')
                MATCH(s_percent_literal)
            MISMATCH
        }

        asterisk:
        {
            ch = read(r);
            if (ch == '*')
                MATCH(s_asterisk_literal)
            MISMATCH
        }

        question:
        {
            ch = read(r);
            if (ch == '?')
                MATCH(s_question_literal)
            MISMATCH
        }

        exponent:
        {
            if (match_literal(r, "**"))
                MATCH(s_exponent_literal)
            MISMATCH
        }

        attribute:
        {
            ch = read(r);
            if (ch == '@') {
                temp = read_identifier(r);
                rewind_one_char();
                if (!temp.empty()) {
                    token.value = temp;
                    token.radix = 10;
                    token.type = token_type_t::attribute;
                    token.number_type = number_types_t::none;
                    MATCH(token)
                }
            }
            MISMATCH
        }

        directive:
        {
            ch = read(r);
            if (ch == '#') {
                temp = read_identifier(r);
                if (!temp.empty()) {
                    token.value = temp;
                    token.radix = 10;
                    token.type = token_type_t::directive;
                    token.number_type = number_types_t::none;
                    MATCH(token)
                }
            }
            MISMATCH
        }

        raw_block:
        {
            if (match_literal(r, "{{")) {
                auto block_count = 1;
                token = s_raw_block;

                stream.str({});
                while (true) {
                    ch = read(r, false);
                    if (ch == common::rune_eof
                    ||  ch == common::rune_invalid) {
                        // XXX: error
                        return false;
                    }

                    if (ch == '{') {
                        ch = read(r, false);
                        if (ch == '{') {
                            block_count++;
                            continue;
                        } else {
                            rewind_one_char();
                            ch = read(r, false);
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
                            ch = read(r, false);
                        }
                    }
                    // XXX: requires utf8 fix
                    stream << static_cast<char>(ch);
                }

                token.value = stream.str();
                MATCH(token)
            }
            MISMATCH
        }

        in_literal:
        {
            if (match_literal(r, "in")) {
                ch = read(r, false);
                if (!isalnum(ch) && ch != '_') {
                    rewind_one_char();
                    MATCH(s_in_literal)
                }
            }
            MISMATCH
        }

        if_literal:
        {
            if (match_literal(r, "if")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_if_literal)
                }
            }
            MISMATCH
        }

        left_paren:
        {
            ch = read(r);
            if (ch == '(') {
                _paren_depth++;
                MATCH(s_left_paren_literal)
            }
            MISMATCH
        }

        identifier:
        {
            if (identifier(r, token))
                MATCH(token)
            MISMATCH
        }

        assignment:
        {
            if (match_literal(r, ":=")) {
                token = _paren_depth == 0 ? s_assignment_literal : s_key_value_operator;
                MATCH(token)
            }
            MISMATCH
        }

        ns_literal:
        {
            if (match_literal(r, "ns")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_namespace_literal)
                }
            }
            MISMATCH
        }

        xor_literal:
        {
            if (match_literal(r, "xor")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_xor_literal)
                }
            }
            MISMATCH
        }

        shl_literal:
        {
            if (match_literal(r, "shl")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_shl_literal)
                }
            }
            MISMATCH
        }

        shr_literal:
        {
            if (match_literal(r, "shr")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_shr_literal)
                }
            }
            MISMATCH
        }

        rol_literal:
        {
            if (match_literal(r, "rol")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_rol_literal)
                }
            }
            MISMATCH
        }

        ror_literal:
        {
            if (match_literal(r, "ror")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_ror_literal);
                }
            }
            MISMATCH
        }

        for_literal:
        {
            if (match_literal(r, "for")) {
                ch = read(r, false);
                if (isspace(ch)) {
                    rewind_one_char();
                    MATCH(s_for_literal)
                }
            }
            MISMATCH
        }

        right_paren:
        {
            ch = read(r);
            if (ch == ')') {
                if (_paren_depth > 0)
                    _paren_depth--;
                MATCH(s_right_paren_literal)
            }
            MISMATCH
        }

        nil_literal:
        {
            if (match_literal(r, "nil")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_nil_literal)
                }
            }
            MISMATCH
        }

        case_literal:
        {
            if (match_literal(r, "case")) {
                ch = read(r, false);
                if (isspace(ch)) {
                    rewind_one_char();
                    MATCH(s_case_literal)
                }
            }
            MISMATCH
        }

        proc_literal:
        {
            if (match_literal(r, "proc")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_proc_literal)
                }
            }
            MISMATCH
        }

        enum_literal:
        {
            if (match_literal(r, "enum")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_enum_literal)
                }
            }
            MISMATCH
        }

        else_literal:
        {
            if (match_literal(r, "else")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_else_literal)
                }
            }
            MISMATCH
        }

        line_comment:
        {
            ch = read(r);
            if (ch == '/') {
                ch = read(r, false);
                if (ch == '/') {
                    token.radix = 10;
                    token.type = token_type_t::line_comment;
                    token.number_type = number_types_t::none;
                    token.value = read_until(r, '\n');
                    MATCH(token)
                }
            }
            MISMATCH
        }

        pipe_literal:
        {
            ch = read(r);
            if (ch == '|')
                MATCH(s_pipe_literal)
            MISMATCH
        }

        true_literal:
        {
            if (match_literal(r, "true")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_true_literal)
                }
            }
            MISMATCH
        }

        with_literal:
        {
            if (match_literal(r, "with")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_with_literal)
                }
            }
            MISMATCH
        }

        from_literal:
        {
            if (match_literal(r, "from")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_from_literal)
                }
            }
            MISMATCH
        }

        block_comment:
        {
            if (match_literal(r, "/*")) {
                auto block_count = 1;
                token = s_block_comment;

                stream.str({});
                while (true) {
                    ch = read(r, false);
                    if (ch == common::rune_eof || ch == common::rune_invalid) {
                        // XXX: error!
                        return false;
                    }

                    if (ch == '/') {
                        ch = read(r, false);
                        if (ch == '*') {
                            block_count++;
                            continue;
                        } else {
                            rewind_one_char();
                            ch = read(r, false);
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
                            ch = read(r, false);
                        }
                    }
                    // XXX: requires utf8 fix
                    stream << static_cast<char>(ch);
                }

                token.value = stream.str();
                MATCH(token)
            }
            MISMATCH
        }

        false_literal:
        {
            if (match_literal(r, "false")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_false_literal)
                }
            }
            MISMATCH
        }

        defer_literal:
        {
            if (match_literal(r, "defer")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_defer_literal)
                }
            }
            MISMATCH
        }

        break_literal:
        {
            if (match_literal(r, "break")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_break_literal)
                }
            }
            MISMATCH
        }

        while_literal:
        {
            if (match_literal(r, "while")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_while_literal)
                }
            }
            MISMATCH
        }

        union_literal:
        {
            if (match_literal(r, "union")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_union_literal)
                }
            }
            MISMATCH
        }

        yield_literal:
        {
            if (match_literal(r, "yield")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_yield_literal)
                }
            }
            MISMATCH
        }

        module_literal:
        {
            if (match_literal(r, "module")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_module_literal)
                }
            }
            MISMATCH
        }

        struct_literal:
        {
            if (match_literal(r, "struct")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_struct_literal)
                }
            }
            MISMATCH
        }

        return_literal:
        {
            if (match_literal(r, "return")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_return_literal)
                }
            }
            MISMATCH
        }

        number_literal:
        {
            auto has_digits = false;

            stream.str({});
            token.radix = 10;
            token.type = token_type_t::number_literal;
            token.number_type = number_types_t::integer;

            ch = read(r);
            if (ch == '$') {
                token.radix = 16;
                has_digits = true;
                while (true) {
                    ch = read(r, false);
                    if (ch == '_')
                        continue;
                    if (!isxdigit(ch))
                        break;
                    // XXX: requires utf8 fix
                    stream << static_cast<char>(ch);
                }
            } else if (ch == '@') {
                token.radix = 8;
                has_digits = true;
                while (true) {
                    ch = read(r, false);
                    if (ch == '_')
                        continue;
                    // XXX: requires utf8 fix
                    if (valid_octal.find_first_of(static_cast<char>(ch)) == std::string::npos)
                        break;
                    // XXX: requires utf8 fix
                    stream << static_cast<char>(ch);
                }
            } else if (ch == '%') {
                token.radix = 2;
                has_digits = true;
                while (true) {
                    ch = read(r, false);
                    if (ch == '_')
                        continue;
                    if (ch != '0' && ch != '1')
                        break;
                    // XXX: requires utf8 fix
                    stream << static_cast<char>(ch);
                }
            } else {
                if (ch == '-') {
                    stream << '-';
                    ch = read(r, false);
                }

                // XXX: requires utf8 fix
                while (valid_decimal.find_first_of(static_cast<char>(ch)) != std::string::npos) {
                    if (ch != '_') {
                        if (ch == '.') {
                            if (token.number_type != number_types_t::floating_point) {
                                token.number_type = number_types_t::floating_point;
                            } else {
                                token.type = token_type_t::invalid;
                                token.number_type = number_types_t::none;
                                has_digits = false;
                                break;
                            }
                        }
                        // XXX: requires utf8 fix
                        stream << static_cast<char>(ch);
                        has_digits = true;
                    }
                    ch = read(r, false);
                }
            }

            token.value = stream.str();
            if (!has_digits || token.value.empty())
                MISMATCH

            rewind_one_char();

            MATCH(token)
        }

        scope_operator:
        {
            if (match_literal(r, "::")) {
                ch = read(r, false);
                if (isalpha(ch) || ch == '_') {
                    rewind_one_char();
                    MATCH(s_scope_operator_literal)
                }
            }
            MISMATCH
        }

        string_literal:
        {
            ch = read(r);
            if (ch == '\"') {
                token.radix = 10;
                token.value = read_until(r, '"');
                token.number_type = number_types_t::none;
                token.type = token_type_t::string_literal;
                MATCH(token)
            }
            MISMATCH
        }

        import_literal:
        {
            if (match_literal(r, "import")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_import_literal)
                }
            }
            MISMATCH
        }

        lambda_literal:
        {
            auto is_lambda = false;

            ch = read(r);
            if (ch == '|') {
                _source_file->push_mark();

                defer({
                    _source_file->restore_top_mark();
                    _source_file->pop_mark();
                });

                while (true) {
                    ch = read(r);
                    if (ch == '|') {
                        is_lambda = true;
                        break;
                    }

                    if (ch == ',')
                        continue;

                    rewind_one_char();
                    if (!identifier(r, temp_token))
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
                        if (!identifier(r, temp_token))
                            break;
                    }
                }
            }

            if (is_lambda) {
                token.type = token_type_t::lambda_literal;
                MATCH(token)
            }

            MISMATCH
        }

        switch_literal:
        {
            if (match_literal(r, "switch")) {
                ch = read(r, false);
                if (isspace(ch)) {
                    rewind_one_char();
                    MATCH(s_switch_literal)
                }
            }
            MISMATCH
        }

        line_terminator:
        {
            ch = read(r);
            if (ch == ';')
                MATCH(s_semi_colon_literal)
            MISMATCH
        }

        equals_operator:
        {
            ch = read(r);
            if (ch == '=') {
                ch = read(r, false);
                if (ch == '=')
                    MATCH(s_equals_literal)
            }
            MISMATCH
        }

        else_if_literal:
        {
            if (match_literal(r, "else if")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_else_if_literal)
                }
            }
            MISMATCH
        }

        left_curly_brace:
        {
            ch = read(r);
            if (ch == '{')
                MATCH(s_left_curly_brace_literal)
            MISMATCH
        }

        continue_literal:
        {
            if (match_literal(r, "continue")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_continue_literal)
                }
            }
            MISMATCH
        }

        right_curly_brace:
        {
            ch = read(r);
            if (ch == '}')
                MATCH(s_right_curly_brace_literal)
            MISMATCH
        }

        ampersand_literal:
        {
            ch = read(r);
            if (ch == '&')
                MATCH(s_ampersand_literal)
            MISMATCH
        }

        character_literal:
        {
            temp.clear();
            uint8_t radix = 10;
            auto is_valid = true;
            auto number_type = number_types_t::none;

            ch = read(r);
            if (ch == '\'') {
                ch = read(r, false);
                if (ch == '\\') {
                    ch = read(r, false);
                    switch (ch) {
                        case 'a': {
                            temp = (char)0x07;
                            break;
                        }
                        case 'b': {
                            temp = (char)0x08;
                            break;
                        }
                        case 'e': {
                            temp = (char)0x1b;
                            break;
                        }
                        case 'n': {
                            temp = (char)0x0a;
                            break;
                        }
                        case 'r': {
                            temp = (char)0x0d;
                            break;
                        }
                        case 't': {
                            temp = (char)0x09;
                            break;
                        }
                        case 'v': {
                            temp = (char)0x0b;
                            break;
                        }
                        case '\\': {
                            temp = "\\";
                            break;
                        }
                        case '\'': {
                            temp = "'";
                            break;
                        }
                        case 'x': {
                            if (!read_hex_digits(r, 2, temp)) {
                                is_valid = false;
                                break;
                            }
                            radix = 16;
                            number_type = number_types_t::integer;
                            break;
                        }
                        case 'u': {
                            if (!read_hex_digits(r, 4, temp)) {
                                is_valid = false;
                                break;
                            }
                            radix = 16;
                            number_type = number_types_t::integer;
                            break;
                        }
                        case 'U': {
                            if (!read_hex_digits(r, 8, temp)) {
                                is_valid = false;
                                break;
                            }
                            radix = 16;
                            number_type = number_types_t::integer;
                            break;
                        }
                        default: {
                            rewind_one_char();
                            if (!read_dec_digits(r, 3, temp)) {
                                is_valid = false;
                                break;
                            }
                            radix = 8;
                            number_type = number_types_t::integer;
                        }
                    }
                } else {
                    temp = static_cast<char>(ch);
                }

                if (is_valid) {
                    ch = read(r);
                    if (ch == '\'') {
                        token.value = temp;
                        token.radix = radix;
                        token.number_type = number_type;
                        token.type = token_type_t::character_literal;
                        MATCH(token)
                    }
                }
            }
            MISMATCH
        }

        value_sink_literal:
        {
            ch = read(r);
            if (ch == '_') {
                ch = read(r, false);
                if (!isalnum(ch) && ch != '_') {
                    rewind_one_char();
                    MATCH(s_value_sink_literal)
                }
            }
            MISMATCH
        }

        less_than_operator:
        {
            ch = read(r);
            if (ch == '<')
                MATCH(s_less_than_literal)
            MISMATCH
        }

        fallthrough_literal:
        {
            if (match_literal(r, "fallthrough")) {
                ch = read(r, false);
                if (!isalnum(ch)) {
                    rewind_one_char();
                    MATCH(s_fallthrough_literal)
                }
            }
            MISMATCH
        }

        not_equals_operator:
        {
            ch = read(r);
            if (ch == '!') {
                ch = read(r);
                if (ch == '=')
                    MATCH(s_not_equals_literal)
            }
            MISMATCH
        }

        left_square_bracket:
        {
            ch = read(r);
            if (ch == '[')
                MATCH(s_left_square_bracket_literal)
            MISMATCH
        }

        logical_or_operator:
        {
            ch = read(r);
            if (ch == '|') {
                ch = read(r);
                if (ch == '|')
                    MATCH(s_logical_or_literal)
            }
            MISMATCH
        }

        constant_assignment:
        {
            if (match_literal(r, "::")) {
                ch = read(r, false);
                if (isspace(ch)) {
                    rewind_one_char();
                    MATCH(s_constant_assignment_literal)
                }
            }
            MISMATCH
        }

        plus_equal_operator:
        {
            if (match_literal(r, "+:="))
                MATCH(s_plus_equal_literal)
            MISMATCH
        }

        logical_and_operator:
        {
            ch = read(r);
            if (ch == '&') {
                ch = read(r);
                if (ch == '&')
                    MATCH(s_logical_and_literal)
            }
            MISMATCH
        }

        right_square_bracket:
        {
            ch = read(r);
            if (ch == ']')
                MATCH(s_right_square_bracket_literal)
            MISMATCH
        }

        minus_equal_operator:
        {
            if (match_literal(r, "-:="))
                MATCH(s_minus_equal_literal)
            MISMATCH
        }

        divide_equal_operator:
        {
            if (match_literal(r, "/:="))
                MATCH(s_divide_equal_literal)
            MISMATCH
        }

        greater_than_operator:
        {
            ch = read(r);
            if (ch == '>')
                MATCH(s_greater_than_literal)
            MISMATCH
        }

        control_flow_operator:
        {
            if (match_literal(r, "=>"))
                MATCH(s_control_flow_operator)
            MISMATCH
        }

        modulus_equal_operator:
        {
            if (match_literal(r, "%:="))
                MATCH(s_modulus_equal_literal)
            MISMATCH
        }

        multiply_equal_operator:
        {
            if (match_literal(r, "*:="))
                MATCH(s_multiply_equal_literal)
            MISMATCH
        }

        less_than_equal_operator:
        {
            if (match_literal(r, "<="))
                MATCH(s_less_than_equal_literal)
            MISMATCH
        }

        binary_or_equal_operator:
        {
            if (match_literal(r, "|:="))
                MATCH(s_binary_or_equal_literal)
            MISMATCH
        }

        binary_not_equal_operator:
        {
            if (match_literal(r, "~:="))
                MATCH(s_binary_not_equal_literal)
            MISMATCH
        }

        binary_and_equal_operator:
        {
            if (match_literal(r, "&:="))
                MATCH(s_binary_and_equal_literal)
            MISMATCH
        }

        greater_than_equal_operator:
        {
            ch = read(r);
            if (ch == '>') {
                ch = read(r);
                if (ch == '=')
                    MATCH(s_greater_than_equal_literal)
            }
            MISMATCH
        }
    }

    common::rune_t lexer::peek(common::result& r) {
        while (!_source_file->eof()) {
            auto ch = _source_file->next(r);
            if (r.is_failed())
                return common::rune_invalid;
            if (!isspace(ch))
                return ch;
        }
        return 0;
    }

    std::string lexer::read_identifier(common::result& r) {
        auto ch = read(r, false);
        if (ch != '_' && !isalpha(ch)) {
            return {};
        }
        std::stringstream stream;
        // XXX: requires utf8 fix
        stream << static_cast<char>(ch);
        while (true) {
            ch = read(r, false);
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

    std::string lexer::read_until(common::result& r, char target_ch) {
        std::stringstream stream;
        while (true) {
            auto ch = read(r, false);
            if (ch == target_ch || ch == -1)
                break;
            // XXX: requires utf8 fix
            stream << static_cast<char>(ch);
        }
        return stream.str();
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

}
