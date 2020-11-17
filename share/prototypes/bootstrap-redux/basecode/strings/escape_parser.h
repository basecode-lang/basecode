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

#include <functional>
#include <string_view>
#include <basecode/utf8/reader.h>

namespace basecode::strings {

    class escape_parser_t final {
    public:
        explicit escape_parser_t(utf8::reader_t& reader);

        bool parse(result_t& r, format::memory_buffer_t& stream);

    private:
        using predicate_t = std::function<bool (utf8::rune_t&)>;

        bool read_digits(
            result_t& r,
            const predicate_t& predicate,
            size_t len,
            std::string_view& value);

    private:
        utf8::reader_t& _reader;
    };

}