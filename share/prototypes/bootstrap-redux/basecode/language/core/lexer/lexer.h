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

#pragma once

#include <utility>
#include <basecode/types.h>
#include <basecode/adt/trie_map.h>
#include <basecode/errors/errors.h>
#include <basecode/adt/hash_table.h>
#include <basecode/workspace/session.h>
#include "token.h"

namespace basecode::language::core::lexer {

    class lexer_t;

    using tokenizer_t = std::function<bool (
        lexer_t*,
        result_t&,
        entity_list_t&)>;

    struct lexeme_t final {
        bool keyword{};
        token_type_t type{};
        tokenizer_t tokenizer{};
    };

    using lexer_trie_t = adt::trie_map_t<lexeme_t*>;

    using entity_maker_t = std::function<entt::entity (
        token_type_t type,
        size_t,
        size_t,
        std::string_view)>;

    using lexeme_init_list_t = std::initializer_list<std::pair<
        std::string_view,
        lexeme_t>>;

    class lexer_t final {
    public:
        lexer_t(
            workspace::session_t& workspace,
            utf8::source_buffer_t& buffer);

        ~lexer_t();

        lexer_trie_t* lexemes();

        bool tokenize(result_t& r, entity_list_t& entities);

    private:
        bool identifier(result_t& r, entity_list_t& entities);

        bool line_comment(result_t& r, entity_list_t& entities);

        bool block_comment(result_t& r, entity_list_t& entities);

        bool string_literal(result_t& r, entity_list_t& entities);

        bool dec_number_literal(result_t& r, entity_list_t& entities);

        bool hex_number_literal(result_t& r, entity_list_t& entities);

        bool octal_number_literal(result_t& r, entity_list_t& entities);

        bool block_string_literal(result_t& r, entity_list_t& entities);

        bool binary_number_literal(result_t& r, entity_list_t& entities);

    private:
        template <typename... Args>
        void add_source_highlighted_error(
                result_t& r,
                errors::error_code_t code,
                size_t start_pos,
                Args&&... args) {
            errors::add_source_highlighted_error(
                r,
                code,
                _buffer,
                make_location(start_pos, _buffer.pos() - start_pos),
                std::forward<Args>(args)...);
        }

        bool make_number_token(
                result_t& r, 
                entity_list_t& entities,
                size_t start_pos,
                bool imaginary,
                bool is_signed, 
                uint8_t radix,
                number_type_t type, 
                std::string_view capture, 
                bool check_sign_bit = true);

        bool tokenize_identifier(
            result_t& r,
            entity_list_t& entities,
            token_type_t token_type,
            const entity_maker_t& entity_maker);

        source_location_t make_location(size_t start_pos, size_t end_pos);

        bool scan_dec_digits(result_t& r, size_t start_pos, number_type_t& type);

    private:
        lexer_trie_t* _lexemes{};
        utf8::source_buffer_t& _buffer;
        workspace::session_t& _session;
        memory::object_pool_t _storage;
        const utf8::source_buffer_line_t* _source_line{};
    };

}
