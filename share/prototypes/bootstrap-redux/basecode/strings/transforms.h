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

#include <basecode/types.h>
#include <basecode/adt/string.h>

namespace basecode::strings {

    using line_callback_t = std::function<bool (std::string_view)>;

    struct unitized_byte_size_t {
        string_t value{};
        string_t units{};
    };

    void word_wrap(
        string_t& text,
        size_t width,
        size_t right_pad = 0,
        const char& fill = ' ');

    string_t pad_to(
        const string_t& str,
        size_t num,
        char padding = ' ');

    bool for_each_line(
        const string_t& buffer,
        const line_callback_t& callback);

    bool string_to_hash_table(
        const string_t& value,
        string_map_t& result_table,
        const char& sep = ',');

    unitized_byte_size_t size_to_units(size_t size);

    string_t remove_underscores(const std::string_view& value);

    string_t list_to_string(const string_list_t& list, const char sep = ',');

    string_list_t string_to_list(const string_t& value, const char sep = ',');

}