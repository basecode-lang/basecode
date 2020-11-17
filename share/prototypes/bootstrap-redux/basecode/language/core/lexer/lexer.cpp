// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
//       C O M P I L E R  P R O J E C T
//
// Copyright (C) 2019 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <basecode/adt/stack.h>
#include <basecode/errors/errors.h>
#include <basecode/numbers/bytes.h>
#include <basecode/numbers/parse.h>
#include <basecode/format/format.h>
#include "lexer.h"

namespace basecode::language::core::lexer {

    lexer_t::lexer_t(
            workspace::session_t& session,
            utf8::source_buffer_t& buffer) : _buffer(buffer),
                                             _session(session),
                                             _storage(session.allocator()) {
    }

    lexer_t::~lexer_t() {
        if (_lexemes != nullptr)
            memory::destroy(_session.allocator(), _lexemes);
    }

    lexer_trie_t* lexer_t::lexemes() {
        if (_lexemes == nullptr) {
            const lexeme_init_list_t lexemes = {
                {"0"sv,           lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::dec_number_literal}},
                {"1"sv,           lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::dec_number_literal}},
                {"2"sv,           lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::dec_number_literal}},
                {"3"sv,           lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::dec_number_literal}},
                {"4"sv,           lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::dec_number_literal}},
                {"5"sv,           lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::dec_number_literal}},
                {"6"sv,           lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::dec_number_literal}},
                {"7"sv,           lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::dec_number_literal}},
                {"8"sv,           lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::dec_number_literal}},
                {"9"sv,           lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::dec_number_literal}},

                {"-1"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::dec_number_literal}},
                {"-2"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::dec_number_literal}},
                {"-3"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::dec_number_literal}},
                {"-4"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::dec_number_literal}},
                {"-5"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::dec_number_literal}},
                {"-6"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::dec_number_literal}},
                {"-7"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::dec_number_literal}},
                {"-8"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::dec_number_literal}},
                {"-9"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::dec_number_literal}},

                {"$0"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::hex_number_literal}},
                {"$1"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::hex_number_literal}},
                {"$2"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::hex_number_literal}},
                {"$3"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::hex_number_literal}},
                {"$4"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::hex_number_literal}},
                {"$5"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::hex_number_literal}},
                {"$6"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::hex_number_literal}},
                {"$7"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::hex_number_literal}},
                {"$8"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::hex_number_literal}},
                {"$9"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::hex_number_literal}},
                {"$a"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::hex_number_literal}},
                {"$A"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::hex_number_literal}},
                {"$b"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::hex_number_literal}},
                {"$B"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::hex_number_literal}},
                {"$c"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::hex_number_literal}},
                {"$C"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::hex_number_literal}},
                {"$d"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::hex_number_literal}},
                {"$D"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::hex_number_literal}},
                {"$e"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::hex_number_literal}},
                {"$E"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::hex_number_literal}},
                {"$f"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::hex_number_literal}},
                {"$F"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::hex_number_literal}},

                {"%0"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::binary_number_literal}},
                {"%1"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::binary_number_literal}},

                {"@0"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::octal_number_literal}},
                {"@1"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::octal_number_literal}},
                {"@2"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::octal_number_literal}},
                {"@3"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::octal_number_literal}},
                {"@4"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::octal_number_literal}},
                {"@5"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::octal_number_literal}},
                {"@6"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::octal_number_literal}},
                {"@7"sv,          lexeme_t{.type = token_type_t::number_literal, .tokenizer = &lexer_t::octal_number_literal}},

                {"\""sv,          lexeme_t{.type = token_type_t::string_literal, .tokenizer = &lexer_t::string_literal}},
                {"{{"sv,          lexeme_t{.type = token_type_t::block_literal, .tokenizer = &lexer_t::block_string_literal}},

                {"//"sv,          lexeme_t{.type = token_type_t::line_comment, .tokenizer = &lexer_t::line_comment}},
                {"--"sv,          lexeme_t{.type = token_type_t::line_comment, .tokenizer = &lexer_t::line_comment}},
                {"/*"sv,          lexeme_t{.type = token_type_t::block_comment, .tokenizer = &lexer_t::block_comment}},

                {":"sv,           lexeme_t{.type = token_type_t::colon}},
                {","sv,           lexeme_t{.type = token_type_t::comma}},
                {"-"sv,           lexeme_t{.type = token_type_t::minus}},
                {"^"sv,           lexeme_t{.type = token_type_t::caret}},
                {"\\"sv,          lexeme_t{.type = token_type_t::backslash}},
                {"<"sv,           lexeme_t{.type = token_type_t::less_than}},
                {";"sv,           lexeme_t{.type = token_type_t::semicolon}},
                {"#"sv,           lexeme_t{.type = token_type_t::directive}},
                {"@"sv,           lexeme_t{.type = token_type_t::annotation}},
                {"("sv,           lexeme_t{.type = token_type_t::left_paren}},
                {")"sv,           lexeme_t{.type = token_type_t::right_paren}},
                {"'"sv,           lexeme_t{.type = token_type_t::single_quote}},
                {">"sv,           lexeme_t{.type = token_type_t::greater_than}},
                {"["sv,           lexeme_t{.type = token_type_t::left_bracket}},
                {"]"sv,           lexeme_t{.type = token_type_t::right_bracket}},
                {"{"sv,           lexeme_t{.type = token_type_t::left_curly_brace}},
                {"}"sv,           lexeme_t{.type = token_type_t::right_curly_brace}},

                {"+"sv,           lexeme_t{.type = token_type_t::add_operator}},
                {"/"sv,           lexeme_t{.type = token_type_t::divide_operator}},
                {"%"sv,           lexeme_t{.type = token_type_t::modulo_operator}},
                {"*"sv,           lexeme_t{.type = token_type_t::multiply_operator}},
                {"**"sv,          lexeme_t{.type = token_type_t::exponent_operator}},
                {"|"sv,           lexeme_t{.type = token_type_t::binary_or_operator}},
                {"~"sv,           lexeme_t{.type = token_type_t::binary_not_operator}},
                {"&"sv,           lexeme_t{.type = token_type_t::binary_and_operator}},
                {"!"sv,           lexeme_t{.type = token_type_t::logical_not_operator}},

                {"=="sv,          lexeme_t{.type = token_type_t::equal_operator}},
                {"!="sv,          lexeme_t{.type = token_type_t::not_equal_operator}},
                {"||"sv,          lexeme_t{.type = token_type_t::logical_or_operator}},
                {"&&"sv,          lexeme_t{.type = token_type_t::logical_and_operator}},
                {"<="sv,          lexeme_t{.type = token_type_t::less_than_equal_operator}},
                {">="sv,          lexeme_t{.type = token_type_t::greater_than_equal_operator}},

                {":="sv,          lexeme_t{.type = token_type_t::assignment_operator}},
                {"+:="sv,         lexeme_t{.type = token_type_t::add_assignment_operator}},
                {"%:="sv,         lexeme_t{.type = token_type_t::modulo_assignment_operator}},
                {"/:="sv,         lexeme_t{.type = token_type_t::divide_assignment_operator}},
                {"*:="sv,         lexeme_t{.type = token_type_t::multiply_assignment_operator}},
                {"-:="sv,         lexeme_t{.type = token_type_t::subtract_assignment_operator}},
                {"|:="sv,         lexeme_t{.type = token_type_t::binary_or_assignment_operator}},
                {"&:="sv,         lexeme_t{.type = token_type_t::binary_and_assignment_operator}},

                {"::"sv,          lexeme_t{.type = token_type_t::bind_operator}},
                {"->"sv,          lexeme_t{.type = token_type_t::lambda_operator}},
                {"..."sv,         lexeme_t{.type = token_type_t::ellipsis_operator}},
                {"=>"sv,          lexeme_t{.type = token_type_t::associative_operator}},
                {"."sv,           lexeme_t{.type = token_type_t::member_select_operator}},
                {".."sv,          lexeme_t{.type = token_type_t::inclusive_range_operator}},
                {"..<"sv,         lexeme_t{.type = token_type_t::exclusive_range_operator}},

                {"in"sv,          lexeme_t{.type = token_type_t::in_operator}},
                {"xor"sv,         lexeme_t{.type = token_type_t::xor_operator}},
                {"shl"sv,         lexeme_t{.type = token_type_t::shl_operator}},
                {"shr"sv,         lexeme_t{.type = token_type_t::shr_operator}},
                {"rol"sv,         lexeme_t{.type = token_type_t::rol_operator}},
                {"ror"sv,         lexeme_t{.type = token_type_t::ror_operator}},

                {"_"sv,           lexeme_t{.keyword = true, .type = token_type_t::value_sink}},
                {"if"sv,          lexeme_t{.keyword = true, .type = token_type_t::if_keyword}},
                {"ns"sv,          lexeme_t{.keyword = true, .type = token_type_t::ns_keyword}},
                {"for"sv,         lexeme_t{.keyword = true, .type = token_type_t::for_keyword}},
                {"nil"sv,         lexeme_t{.keyword = true, .type = token_type_t::nil_keyword}},
                {"use"sv,         lexeme_t{.keyword = true, .type = token_type_t::use_keyword}},
                {"true"sv,        lexeme_t{.keyword = true, .type = token_type_t::true_keyword}},
                {"cast"sv,        lexeme_t{.keyword = true, .type = token_type_t::cast_keyword}},
                {"case"sv,        lexeme_t{.keyword = true, .type = token_type_t::case_keyword}},
                {"proc"sv,        lexeme_t{.keyword = true, .type = token_type_t::proc_keyword}},
                {"enum"sv,        lexeme_t{.keyword = true, .type = token_type_t::enum_keyword}},
                {"else"sv,        lexeme_t{.keyword = true, .type = token_type_t::else_keyword}},
                {"with"sv,        lexeme_t{.keyword = true, .type = token_type_t::with_keyword}},
                {"goto"sv,        lexeme_t{.keyword = true, .type = token_type_t::goto_keyword}},
                {"false"sv,       lexeme_t{.keyword = true, .type = token_type_t::false_keyword}},
                {"defer"sv,       lexeme_t{.keyword = true, .type = token_type_t::defer_keyword}},
                {"break"sv,       lexeme_t{.keyword = true, .type = token_type_t::break_keyword}},
                {"union"sv,       lexeme_t{.keyword = true, .type = token_type_t::union_keyword}},
                {"yield"sv,       lexeme_t{.keyword = true, .type = token_type_t::yield_keyword}},
                {"?"sv,           lexeme_t{.keyword = true, .type = token_type_t::uninitialized}},
                {"struct"sv,      lexeme_t{.keyword = true, .type = token_type_t::struct_keyword}},
                {"return"sv,      lexeme_t{.keyword = true, .type = token_type_t::return_keyword}},
                {"switch"sv,      lexeme_t{.keyword = true, .type = token_type_t::switch_keyword}},
                {"family"sv,      lexeme_t{.keyword = true, .type = token_type_t::family_keyword}},
                {"module"sv,      lexeme_t{.keyword = true, .type = token_type_t::module_keyword}},
                {"import"sv,      lexeme_t{.keyword = true, .type = token_type_t::import_keyword}},
                {"else if"sv,     lexeme_t{.keyword = true, .type = token_type_t::else_if_keyword}},
                {"bitcast"sv,     lexeme_t{.keyword = true, .type = token_type_t::bitcast_keyword}},
                {"continue"sv,    lexeme_t{.keyword = true, .type = token_type_t::continue_keyword}},
                {"fallthrough"sv, lexeme_t{.keyword = true, .type = token_type_t::fallthrough_keyword}},
            };

            _lexemes = memory::construct<lexer_trie_t>(_session.allocator());
            for (const auto& e : lexemes) {
                auto lexeme = _storage.construct<lexeme_t>();
                lexeme->type = e.second.type;
                lexeme->tokenizer = e.second.tokenizer;
                _lexemes->insert(e.first, lexeme);
            }
        }

        return _lexemes;
    }

    bool lexer_t::make_number_token(
            result_t& r,
            entity_list_t& entities,
            size_t start_pos,
            bool imaginary,
            bool is_signed,
            uint8_t radix,
            number_type_t type,
            std::string_view capture,
            bool check_sign_bit) {
        auto& registry = _session.registry();

        auto token = registry.create();
        registry.assign<token_t>(token, token_type_t::number_literal, capture);

        auto& number_token = registry.assign<number_token_t>(
            token,
            is_signed,
            imaginary,
            radix,
            type,
            number_size_t::qword);

        if (type == number_type_t::integer) {
            int64_t value;
            auto result = numbers::parse_integer(capture, radix, value);
            if (result != numbers::conversion_result_t::success) {
                add_source_highlighted_error(
                    r,
                    errors::lexer::unable_to_convert_integer_value,
                    start_pos,
                    capture,
                    numbers::conversion_result_to_name(result));
                return false;
            }

            auto narrowed_size = narrow_type(value);
            if (!narrowed_size) {
                add_source_highlighted_error(
                    r,
                    errors::lexer::unable_to_narrow_integer_value,
                    start_pos);
                return false;
            }

            apply_narrowed_value(number_token, *narrowed_size, value, check_sign_bit);
        } else if (type == number_type_t::floating_point) {
            double value;

            if (imaginary)
                capture = std::string_view(capture.data(), capture.length() - 1);

            auto result = numbers::parse_double(capture, value);
            if (result != numbers::conversion_result_t::success) {
                add_source_highlighted_error(
                    r,
                    errors::lexer::unable_to_convert_floating_point_value,
                    start_pos,
                    capture,
                    numbers::conversion_result_to_name(result));
                return false;
            }

            auto narrowed_size = narrow_type(value);
            if (!narrowed_size) {
                add_source_highlighted_error(
                    r,
                    errors::lexer::unable_to_narrow_floating_point_value,
                    start_pos);
                return false;
            }

            apply_narrowed_value(number_token, *narrowed_size, value);
        }

        registry.assign<source_location_t>(
            token,
            make_location(start_pos, _buffer.pos()));
        entities.add(token);

        return true;
    }

    bool lexer_t::tokenize_identifier(
            result_t& r,
            entity_list_t& entities,
            token_type_t token_type,
            const entity_maker_t& entity_maker) {
        auto start_pos = _buffer.pos();

        auto rune = _buffer.curr(r);
        if (rune.is_errored())
            return false;

        if (rune != '_' && !rune.is_alpha()) {
            add_source_highlighted_error(
                r,
                errors::lexer::invalid_identifier_start_character,
                start_pos,
                rune);
            return false;
        }

        if (!_buffer.move_next(r))
            return false;

        while (true) {
            rune = _buffer.curr(r);
            if (rune.is_errored())
                return false;

            if (!rune.is_digit()
            &&  !rune.is_alpha()
            &&  rune != '_') {
                break;
            }

            if (!_buffer.move_next(r))
                return false;
        }

        auto end_pos = _buffer.pos();
        auto capture = _buffer.make_slice(start_pos, end_pos - start_pos);

        return entity_maker(token_type, start_pos, end_pos, capture) != entt::null;
    }

    bool lexer_t::tokenize(result_t& r, entity_list_t& entities) {
        auto& registry = _session.registry();
        auto local_lexemes = lexemes();
        while (!_buffer.eof()) {
            lexer_trie_t::node_t* current_node = nullptr;
            lexeme_t* matched_lexeme = nullptr;

            auto rune = _buffer.curr(r);
            if (rune.is_errored())
                return false;

            if (rune.is_space()) {
                if (!_buffer.move_next(r))
                    break;
                continue;
            }

            _buffer.push_mark();

            while (true) {
                current_node = local_lexemes->find(current_node, rune);
                if (current_node == nullptr) {
                    if (matched_lexeme
                    &&  matched_lexeme->keyword) {
                        if (rune.is_alpha() || rune == '_')
                            matched_lexeme = nullptr;
                    }
                    break;
                }

                if (current_node->data != nullptr)
                    matched_lexeme = current_node->data;

                if (!_buffer.move_next(r))
                    return false;

                rune = _buffer.curr(r);
                if (rune.is_errored())
                    return false;
            }

            if (matched_lexeme == nullptr) {
                _buffer.restore_top_mark();
                auto start_pos = _buffer.pop_mark();

                if (!identifier(r, entities)) {
                    add_source_highlighted_error(
                        r,
                        errors::lexer::expected_identifier,
                        start_pos);
                    return false;
                }
            } else {
                if (matched_lexeme->tokenizer) {
                    _buffer.restore_top_mark();
                    _buffer.pop_mark();
                    if (!matched_lexeme->tokenizer(this, r, entities))
                        return false;
                } else {
                    auto start_pos = _buffer.pop_mark();
                    auto end_pos = _buffer.pos();
                    auto token = registry.create();
                    registry.assign<token_t>(
                        token,
                        matched_lexeme->type,
                        _buffer.make_slice(start_pos, end_pos - start_pos));
                    registry.assign<source_location_t>(
                        token,
                        make_location(start_pos, end_pos));
                    entities.add(token);
                }
            }
        }

        auto token = registry.create();
        registry.assign<token_t>(token, token_type_t::end_of_input);
        registry.assign<source_location_t>(
            token,
            make_location(_buffer.pos(), _buffer.pos()));
        entities.add(token);

        return !r.is_failed();
    }

    bool lexer_t::identifier(result_t& r, entity_list_t& entities) {
        return tokenize_identifier(
            r,
            entities,
            token_type_t::identifier,
            [&](auto token_type, auto start_pos, auto end_pos, auto capture) {
                auto& registry = _session.registry();
                auto token = registry.create();
                registry.assign<token_t>(token, token_type, capture);
                registry.assign<source_location_t>(
                    token,
                    make_location(start_pos, end_pos));
                entities.add(token);
                return token;
            });
    }

    bool lexer_t::line_comment(result_t& r, entity_list_t& entities) {
        // prefixed with // or --
        if (!_buffer.move_next(r)) return false;
        if (!_buffer.move_next(r)) return false;
        
        auto start_pos = _buffer.pos();
        while (true) {
            auto rune = _buffer.curr(r);
            if (rune.is_errored())
                return false;
            if (rune == '\n') break;
            if (!_buffer.move_next(r))
                return false;
        }

        auto capture = _buffer.make_slice(
            start_pos,
            _buffer.pos() - start_pos);

        auto& registry = _session.registry();
        auto token = registry.create();
        registry.assign<token_t>(token, token_type_t::line_comment, capture);
        registry.assign<source_location_t>(
            token,
            make_location(start_pos, _buffer.pos()));
        entities.add(token);

        return true;
    }

    bool lexer_t::block_comment(result_t& r, entity_list_t& entities) {
        // prefixed with /*
        if (!_buffer.move_next(r)) return false;
        if (!_buffer.move_next(r)) return false;

        auto block_count = 1;
        auto start_pos = _buffer.pos();

        adt::stack_t<size_t> block_starts(_session.allocator());
        block_starts.push(start_pos);

        adt::stack_t<block_comment_token_t*> blocks(_session.allocator());

        block_comment_token_t root(_session.allocator());
        auto* current_block = &root;

        while (true) {
            auto rune = _buffer.curr(r);
            if (rune.is_errored())
                return false;

            if (rune == '/') {
                if (!_buffer.move_next(r)) 
                    return false;
                rune = _buffer.curr(r);

                if (rune == '*') {
                    ++block_count;

                    block_starts.push(_buffer.pos() + 1);
                    blocks.push(current_block);

                    current_block = &current_block->children.emplace(_session.allocator());
                } else {
                    continue;
                }
            } else if (rune == '*') {
                if (!_buffer.move_next(r)) 
                    return false;
                rune = _buffer.curr(r);

                if (rune == '/') {
                    if (block_count > 0) {
                        --block_count;

                        auto block_start = *block_starts.top();
                        block_starts.pop();

                        current_block->capture = _buffer.make_slice(
                            block_start,
                            _buffer.pos() - block_start - 1);

                        if (!blocks.empty()) {
                            current_block = blocks.top();
                            blocks.pop();
                        }
                    }
                    if (block_count == 0) { 
                        if (!_buffer.move_next(r)) 
                            return false;
                        break;
                    }
                } else {
                    continue;
                }
            }

            if (!_buffer.move_next(r))
                return false;
        }

        auto end_pos = _buffer.pos() - 2;
        auto capture = _buffer.make_slice(start_pos, end_pos - start_pos);

        auto& registry = _session.registry();
        auto token = registry.create();
        registry.assign<token_t>(token, token_type_t::block_comment, capture);
        auto& comment_token = registry.assign<block_comment_token_t>(
            token,
            _session.allocator());
        comment_token.capture = root.capture;
        comment_token.children = std::move(root.children);
        registry.assign<source_location_t>(
            token,
            make_location(start_pos, end_pos));
        entities.add(token);

        return true;
    }

    bool lexer_t::string_literal(result_t& r, entity_list_t& entities) {
        // prefixed with "
        if (!_buffer.move_next(r)) return false;
        
        auto should_exit = false;
        auto start_pos = _buffer.pos();
        while (!should_exit) {
            auto rune = _buffer.curr(r);
            if (rune.is_errored())
                return false;
            if (rune == '\\') {
                if (!_buffer.move_next(r))
                    return false;

                rune = _buffer.curr(r);
                if (rune.is_errored())
                    return false;

                auto extra_chars = 0;

                switch ((int32_t)rune) {
                    case 'a':
                    case 'b':
                    case 'e':
                    case 'n':
                    case 'r':
                    case 't':
                    case 'v':
                    case '\\':
                    case '\"':
                    case '\'':
                        if (!_buffer.move_next(r))
                            return false;
                        break;
                    case 'x':
                        if (!_buffer.move_next(r))
                            return false;
                        extra_chars = 2;
                        break;
                    case 'u':
                        if (!_buffer.move_next(r))
                            return false;
                        extra_chars = 4;
                        break;
                    case 'U':
                        if (!_buffer.move_next(r))
                            return false;
                        extra_chars = 8;
                        break;
                    default:
                        extra_chars = 3;
                        break;
                }

                for (size_t i = 0; i < extra_chars; i++)
                    if (!_buffer.move_next(r))
                        return false;

                continue;
            } else if (rune == '\"') {
                should_exit = true;
            }
            if (!_buffer.move_next(r))
                return false;
        }

        auto rune = _buffer.curr(r);
        if (rune.is_errored())
            return false;
        if (rune == '\"') {
            add_source_highlighted_error(
                r,
                errors::lexer::unescaped_quote,
                start_pos);
            return false;
        }

        auto end_pos = _buffer.pos() - 1;
        auto capture = _buffer.make_slice(start_pos, end_pos - start_pos);

        auto& registry = _session.registry();

        auto token = registry.create();
        registry.assign<token_t>(token, token_type_t::string_literal, capture);
        registry.assign<source_location_t>(
            token,
            make_location(start_pos, end_pos));
        entities.add(token);

        return true;
    }

    bool lexer_t::dec_number_literal(result_t& r, entity_list_t& entities) {
        bool imaginary{};
        auto type = number_type_t::integer;

        auto rune = _buffer.curr(r);
        bool is_signed = rune == '-';
        if (is_signed) {
            if (!_buffer.move_next(r))
                return false;
        }

        auto start_pos = _buffer.pos();

        if (!scan_dec_digits(r, start_pos, type))
            return false;

        rune = _buffer.curr(r);
        if (rune.is_errored())
            return false;

        if (rune == 'e' || rune == 'E') {
            if (type != number_type_t::floating_point) {
                add_source_highlighted_error(
                    r,
                    errors::lexer::exponent_notation_not_valid_for_integers,
                    start_pos);
                return false;
            }

            if (!_buffer.move_next(r))
                return false;

            rune = _buffer.curr(r);
            if (rune.is_errored())
                return false;

            if (rune == '-' || rune == '+') {
                if (!_buffer.move_next(r))
                    return false;

                rune = _buffer.curr(r);
                if (rune.is_errored())
                    return false;
            }

            if (!scan_dec_digits(r, _buffer.pos(), type))
                return false;

            rune = _buffer.curr(r);
            if (rune.is_errored())
                return false;

            if (rune == 'i') {
                if (!_buffer.move_next(r))
                    return false;
                imaginary = true;
            }
        } else if (rune.is_alpha()) {
            add_source_highlighted_error(
                r,
                errors::lexer::unexpected_letter_after_decimal_number_literal,
                start_pos);
            return false;
        }

        auto capture = _buffer.make_slice(
            start_pos,
            _buffer.pos() - start_pos);

        return make_number_token(
                r,
                entities, 
                start_pos,
                imaginary,
                is_signed, 
                10, 
                type, 
                capture, 
                false);
    }

    bool lexer_t::hex_number_literal(result_t& r, entity_list_t& entities) {
        auto start_pos = _buffer.pos();

        auto rune = _buffer.next(r);
        if (rune != '$') {
            add_source_highlighted_error(
                r,
                errors::lexer::expected_hex_literal_prefix,
                start_pos);
            return false;
        }

        start_pos = _buffer.pos();
        while (true) {
            rune = _buffer.curr(r);
            if (rune.is_errored())
                return false;
            if (rune == '_') {
                if (!_buffer.move_next(r))
                    return false;
                continue;
            }
            if (!rune.is_xdigit()) {
                if (rune.is_alpha()) {
                    add_source_highlighted_error(
                        r,
                        errors::lexer::unexpected_letter_after_hexadecimal_number_literal,
                        start_pos);
                    return false;
                }
                break;
            }
            if (!_buffer.move_next(r))  return false;
        }

        auto capture = _buffer.make_slice(
            start_pos,
            _buffer.pos() - start_pos);

        return make_number_token(
                r, 
                entities, 
                start_pos,
                false,
                false, 
                16, 
                number_type_t::integer, 
                capture);
    }

    bool lexer_t::octal_number_literal(result_t& r, entity_list_t& entities) {
        auto start_pos = _buffer.pos();

        auto rune = _buffer.next(r);
        if (rune != '@') {
            add_source_highlighted_error(
                r,
                errors::lexer::expected_octal_literal_prefix,
                start_pos);
            return false;
        }

        start_pos = _buffer.pos();
        while (true) {
            rune = _buffer.curr(r);
            if (rune.is_errored())
                return false;
            if (rune == '_') {
                if (!_buffer.move_next(r))
                    return false;
                continue;
            }
            if (rune < 0x30 || rune > 0x37) {
                if (rune.is_alpha()) {
                    add_source_highlighted_error(
                        r,
                        errors::lexer::unexpected_letter_after_octal_number_literal,
                        start_pos);
                    return false;
                }
                break;
            }
            if (!_buffer.move_next(r))  return false;
        }

        auto capture = _buffer.make_slice(
            start_pos,
            _buffer.pos() - start_pos);

        return make_number_token(
                r, 
                entities, 
                start_pos,
                false,
                false, 
                8, 
                number_type_t::integer, capture);
    }

    bool lexer_t::binary_number_literal(result_t& r, entity_list_t& entities) {
        auto start_pos = _buffer.pos();

        auto rune = _buffer.next(r);
        if (rune != '%') {
            add_source_highlighted_error(
                r,
                errors::lexer::expected_binary_literal_prefix,
                start_pos);
            return false;
        }

        start_pos = _buffer.pos();
        while (true) {
            rune = _buffer.curr(r);
            if (rune.is_errored())
                return false;
            if (rune == '_') {
                if (!_buffer.move_next(r))
                    return false;
                continue;
            }
            if (rune < 0x30 || rune > 0x31) {
                if (rune.is_alpha() || rune.is_digit()) {
                    add_source_highlighted_error(
                        r,
                        errors::lexer::unexpected_letter_after_binary_number_literal,
                        start_pos);
                    return false;
                }
                break;
            }
            if (!_buffer.move_next(r))  return false;
        }

        auto capture = _buffer.make_slice(
            start_pos,
            _buffer.pos() - start_pos);

        return make_number_token(
                r, 
                entities, 
                start_pos,
                false,
                false, 
                2, 
                number_type_t::integer, 
                capture);
    }

    bool lexer_t::block_string_literal(result_t& r, entity_list_t& entities) {
        // prefixed with {{
        if (!_buffer.move_next(r)) return false;
        if (!_buffer.move_next(r)) return false;

        auto start_pos = _buffer.pos();
        while (true) {
            auto rune = _buffer.curr(r);
            if (rune.is_errored())
                return false;
            if (rune == '}') {
                if (!_buffer.move_next(r))
                    return false;

                rune = _buffer.curr(r);
                if (rune.is_errored())
                    return false;

                if (rune != '}') {
                    add_source_highlighted_error(
                        r,
                        errors::lexer::expected_closing_block_literal,
                        start_pos,
                        rune);
                    return false;
                }

                break;
            }
            if (!_buffer.move_next(r))
                return false;
        }

        auto end_pos = _buffer.pos() - 2;
        auto capture = _buffer.make_slice(start_pos, end_pos - start_pos);

        auto& registry = _session.registry();
        auto token = registry.create();
        registry.assign<token_t>(token, token_type_t::block_literal, capture);
        registry.assign<source_location_t>(
            token,
            make_location(start_pos, end_pos));
        entities.add(token);

        return true;
    }

    source_location_t lexer_t::make_location(size_t start_pos, size_t end_pos) {
        if (_source_line == nullptr
        ||  start_pos > _source_line->end) {
            _source_line = _buffer.line_by_index(start_pos);
        }
        auto end_line = _source_line;
        if (end_pos > end_line->end) {
            end_line = _buffer.line_by_index(end_pos);
        }

        return {
            {end_line->line, end_line->column(end_pos)},
            {_source_line->line, _source_line->column(start_pos)}
        };
    }

    bool lexer_t::scan_dec_digits(result_t& r, size_t start_pos, number_type_t& type) {
        while (true) {
            auto rune = _buffer.curr(r);

            if (rune.is_errored())
                return false;

            if (rune == '.') {
                if (type == number_type_t::floating_point) {
                    add_source_highlighted_error(
                        r,
                        errors::lexer::unexpected_decimal_point,
                        start_pos);
                    return false;
                } else {
                    type = number_type_t::floating_point;
                }
            } else if (rune == '_') {
                // N.B. ignore
            } else if (rune < 0x30 || rune > 0x39) {
                break;
            }
            if (!_buffer.move_next(r))   return false;
        }

        return true;
    }

}
