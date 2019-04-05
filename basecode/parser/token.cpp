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

#include "token.h"

namespace basecode::syntax {

    bool token_t::as_bool() const {
        return value == "true";
    }

    bool token_t::is_label() const {
        return type == token_type_t::label;
    }

    bool token_t::is_boolean() const {
        return type == token_type_t::true_literal
               || type == token_type_t::false_literal;
    }

    bool token_t::is_numeric() const {
        return type == token_type_t::number_literal;
    }

    bool token_t::is_signed() const {
        return !value.empty() && value[0] == '-';
    }

    std::string token_t::name() const {
        auto it = s_type_to_name.find(type);
        if (it == s_type_to_name.end())
            return "unknown";
        return it->second;
    }

    bool token_t::is_line_comment() const {
        return type == token_type_t::line_comment;
    }

    bool token_t::is_block_comment() const {
        return type == token_type_t::block_comment;
    }

    conversion_result_t token_t::parse(double& out) const {
        const char* s = value.c_str();
        char* end;
        errno = 0;
        out = strtod(s, &end);
        if (errno == ERANGE)
            return conversion_result_t::overflow;
        if (*s == '\0' || *end != '\0')
            return conversion_result_t::inconvertible;
        return conversion_result_t::success;
    }

    conversion_result_t token_t::parse(int64_t& out) const {
        const char* s = value.c_str();
        char* end;
        errno = 0;
        out = strtoll(s, &end, radix);
        if ((errno == ERANGE && out == LONG_MAX)
        ||   out > UINT_MAX)
            return conversion_result_t::overflow;
        if ((errno == ERANGE && out == LONG_MIN))
            return conversion_result_t::underflow;
        if (*s == '\0' || *end != '\0')
            return conversion_result_t::inconvertible;
        return conversion_result_t::success;
    }

    conversion_result_t token_t::parse(uint64_t& out) const {
        if (value.empty())
            return conversion_result_t::inconvertible;
        const char* s = nullptr;
        std::string without_negative;
        if (value[0] == '-') {
            without_negative = value.substr(1);
            s = without_negative.c_str();
        }
        else
            s = value.c_str();
        char* end;
        errno = 0;
        out = strtoull(s, &end, radix);
        if (errno == ERANGE)
            return conversion_result_t::overflow;
        if (*s == '\0' || *end != '\0')
            return conversion_result_t::inconvertible;
        return conversion_result_t::success;
    }

}
