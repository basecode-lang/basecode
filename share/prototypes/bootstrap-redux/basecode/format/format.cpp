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

#include "format.h"

namespace basecode::format {

    void vprint(
            allocator_t alloc,
            FILE* file,
            fmt::string_view format_str,
            fmt::format_args args) {
        memory_buffer_t buffer(alloc);
        fmt::vformat_to(buffer, format_str, args);
        std::fwrite(buffer.data(), 1, buffer.size(), file);
    }

    string_t vformat(
            allocator_t alloc,
            fmt::string_view format_str,
            fmt::format_args args) {
        memory_buffer_t buffer(alloc);
        fmt::vformat_to(buffer, format_str, args);
        return std::string_view(buffer.data(), buffer.size());
    }

    string_t to_string(const memory_buffer_t& buf) {
        return std::string_view(buf.data(), buf.size());
    }

}
