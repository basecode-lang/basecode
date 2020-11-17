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
#include <basecode/memory/std_allocator.h>
#include "formatters.h"

namespace basecode::format {

    using allocator_t = memory::std_allocator_t<char>;
    using memory_buffer_t = fmt::basic_memory_buffer<
        char,
        fmt::inline_buffer_size,
        allocator_t>;

    void vprint(
        allocator_t alloc,
        FILE* file,
        fmt::string_view format_str,
        fmt::format_args args);

    string_t vformat(
        allocator_t alloc,
        fmt::string_view format_str,
        fmt::format_args args);

    string_t to_string(const memory_buffer_t& buf);

    template <typename... Args>
    inline void print(
            fmt::string_view format_str,
            const Args&... args) {
        vprint(allocator_t{}, stdout, format_str, fmt::make_format_args(args...));
    }

    template <typename... Args>
    inline void print(
            allocator_t alloc,
            fmt::string_view format_str,
            const Args&... args) {
        vprint(alloc, stdout, format_str, fmt::make_format_args(args...));
    }

    template <typename... Args>
    inline string_t format(
            allocator_t alloc,
            fmt::string_view format_str,
            const Args&... args) {
        return vformat(alloc, format_str, fmt::make_format_args(args...));
    }

    template <typename... Args>
    inline string_t format(
            fmt::string_view format_str,
            const Args&... args) {
        return vformat(allocator_t{}, format_str, fmt::make_format_args(args...));
    }

    template <typename S, typename... Args>
    inline fmt::buffer_context<char>::type::iterator format_to(
            memory_buffer_t& buf,
            const S& format_str,
            const Args&... args) {
        return fmt::vformat_to(buf, format_str, fmt::make_format_args(args...));
    }

}