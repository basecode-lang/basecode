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

#include <string_view>

namespace basecode::numbers {

    using namespace std::literals;

    enum class conversion_result_t {
        success,
        overflow,
        underflow,
        not_convertible
    };

    std::string_view conversion_result_to_name(conversion_result_t type);

    conversion_result_t parse_double(std::string_view value, double& out);

    conversion_result_t parse_integer(std::string_view value, uint8_t radix, int32_t& out);

    conversion_result_t parse_integer(std::string_view value, uint8_t radix, int64_t& out);

}