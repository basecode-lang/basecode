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
#include <deque>
#include <vector>
#include <istream>
#include <functional>
#include <common/rune.h>
#include <common/source_file.h>
#include "token.h"

namespace basecode::syntax {

    class lexer {
    public:
        explicit lexer(common::source_file* source_file);

        bool has_next() const;

        bool next(token_t& token);

        bool tokenize(common::result& r);

    private:
        bool identifier(
            common::result& r,
            token_t& token);

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

        void rewind_one_char();

        bool naked_identifier(
            common::result& r,
            token_t& token);

        bool type_tagged_identifier(
            common::result& r,
            token_t& token);

        common::rune_t peek(common::result& r);

        std::string read_identifier(common::result& r);

        std::string read_until(common::result& r, char target_ch);

    private:
        size_t _index = 0;
        bool _has_next = true;
        uint32_t _paren_depth = 0;
        std::vector<token_t> _tokens {};
        common::source_file* _source_file = nullptr;
    };

}

