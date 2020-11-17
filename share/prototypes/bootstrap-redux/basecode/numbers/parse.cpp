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

#include <basecode/strings/transforms.h>
#include "parse.h"

namespace basecode::numbers {

    std::string_view conversion_result_to_name(conversion_result_t type) {
        switch (type) {
            case conversion_result_t::success:          return "success"sv;
            case conversion_result_t::overflow:         return "overflow"sv;
            case conversion_result_t::underflow:        return "underflow"sv;
            case conversion_result_t::not_convertible:  return "not convertible"sv;
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    conversion_result_t parse_double(std::string_view value, double& out) {
        const char* s = strings::remove_underscores(value).c_str();
        char* end;
        errno = 0;
        out = strtod(s, &end);
        if (errno == ERANGE)
            return conversion_result_t::overflow;
        if (*s == '\0' || *end != '\0')
            return conversion_result_t::not_convertible;
        return conversion_result_t::success;
    }

    conversion_result_t parse_integer(std::string_view value, uint8_t radix, int32_t& out) {
        const char* s = strings::remove_underscores(value).c_str();
        char* end;
        errno = 0;
        out = strtol(s, &end, radix);
        if ((errno == ERANGE && out == INT_MAX)
        ||   out > UINT_MAX)
            return conversion_result_t::overflow;
        if ((errno == ERANGE && out == INT_MIN))
            return conversion_result_t::underflow;
        if (*s == '\0' || *end != '\0')
            return conversion_result_t::not_convertible;
        return conversion_result_t::success;
    }

    conversion_result_t parse_integer(std::string_view value, uint8_t radix, int64_t& out) {
        const char* s = strings::remove_underscores(value).c_str();
        char* end;
        errno = 0;
        out = strtoll(s, &end, radix);
        if ((errno == ERANGE && out == LONG_MAX)
        ||   out > UINT_MAX)
            return conversion_result_t::overflow;
        if ((errno == ERANGE && out == LONG_MIN))
            return conversion_result_t::underflow;
        if (*s == '\0' || *end != '\0')
            return conversion_result_t::not_convertible;
        return conversion_result_t::success;
    }

}