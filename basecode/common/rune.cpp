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

#include "rune.h"

namespace basecode::common {

    bool is_rune_digit(rune_t r) {
        if (r < 0x80) {
            return isdigit(r) != 0;
        }
        return utf8proc_category(r) == UTF8PROC_CATEGORY_ND;
    }

    bool is_rune_letter(rune_t r) {
        if (r < 0x80) {
            if (r == '_') return true;
            return isalpha(r) != 0;
        }
        switch (utf8proc_category(r)) {
            case UTF8PROC_CATEGORY_LU:
            case UTF8PROC_CATEGORY_LL:
            case UTF8PROC_CATEGORY_LT:
            case UTF8PROC_CATEGORY_LM:
            case UTF8PROC_CATEGORY_LO:
                return true;
            default:
                break;
        }
        return false;
    }

    bool is_rune_whitespace(rune_t r) {
        switch (r) {
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                return true;
            default:
                return false;
        }
    }

    encoded_rune_t utf8_encode(rune_t r) {
        encoded_rune_t e {};

        auto i = static_cast<uint32_t>(r);
        uint8_t mask = 0x3f;
        if (i <= (1<<7)-1) {
            e.data[0] = static_cast<uint8_t>(r);
            e.width = 1;
            return e;
        }

        if (i <= (1<<11)-1) {
            e.data[0] = static_cast<uint8_t>(0xc0 | static_cast<uint8_t>((r >> 6)));
            e.data[1] = static_cast<uint8_t>(0x80 | (static_cast<uint8_t>((r)) & mask));
            e.width = 2;
            return e;
        }

        if (i > rune_max || (i >= 0xd800 && i <= 0xdfff)) {
            e.data[0] = static_cast<uint8_t>(0xe0 | (uint8_t)(r >> 12));
            e.data[1] = static_cast<uint8_t>(0x80 | ((uint8_t)(r >> 6) & mask));
            e.data[2] = static_cast<uint8_t>(0x80 | ((uint8_t)(r) & mask));
            e.value = rune_invalid;
            e.width = 3;
            return e;
        }

        if (i <= (1<<16)-1) {
            e.data[0] = static_cast<uint8_t>(0xe0 | (uint8_t)(r >> 12));
            e.data[1] = static_cast<uint8_t>(0x80 | ((uint8_t)(r >> 6) & mask));
            e.data[2] = static_cast<uint8_t>(0x80 | ((uint8_t)(r) & mask));
            e.width = 3;
            return e;
        }

        e.data[0] = static_cast<uint8_t>(0xf0 | (uint8_t)(r >> 18));
        e.data[1] = static_cast<uint8_t>(0x80 | ((uint8_t)(r >> 12) & mask));
        e.data[2] = static_cast<uint8_t>(0x80 | ((uint8_t)(r >> 6) & mask));
        e.data[3] = static_cast<uint8_t>(0x80 | ((uint8_t)(r) & mask));
        e.width = 4;

        return e;
    }

    int64_t utf8_strlen(const std::string& str) {
        int64_t len = 0;
        auto p = str.data();
        for (; *p; len++) {
            auto c = static_cast<uint8_t>(*p);

            size_t cp_size = 0;
            if (c < 0x80)
                cp_size = 1;
            else if ((c & 0xe0) == 0xc0)
                cp_size = 2;
            else if ((c & 0xf0) == 0xe0)
                cp_size = 3;
            else if ((c & 0xf8) == 0xf0)
                cp_size = 4;
            else
                return -1;

            p += cp_size;
        }
        return len;
    }

    codepoint_t utf8_decode(char* str, size_t length) {
        codepoint_t cp {};
        if (length == 0)
            return cp;

        auto s0 = (uint8_t) str[0];
        uint8_t x = s_utf8_first[s0], sz;
        uint8_t b1, b2, b3;
        utf8_accept_range_t accept {};

        if (x >= 0xf0) {
            rune_t mask = (static_cast<rune_t>(x) << 31) >> 31;
            cp.value = (static_cast<rune_t>(s0) & (~mask)) | (rune_invalid & mask);
            cp.width = 1;
            return cp;
        }

        if (s0 < 0x80) {
            cp.value = s0;
            cp.width = 1;
            return cp;
        }

        sz = static_cast<uint8_t>(x & 7);
        accept = s_utf8_accept_ranges[x>>4];
        if (length < sizeof(sz))
            return cp;

        b1 = (uint8_t) str[1];
        if (b1 < accept.low || accept.high < b1)
            return cp;

        if (sz == 2) {
            cp.value = (static_cast<rune_t>(s0)&0x1f)<<6
                       | (static_cast<rune_t>(b1)&0x3f);
            cp.width = 2;
            return cp;
        }

        b2 = (uint8_t) str[2];
        if (!(b2 >= 0x80 && b2 <= 0xbf))
            return cp;

        if (sz == 3) {
            cp.value = (static_cast<rune_t>(s0)&0x1f)<<12
                       | (static_cast<rune_t>(b1)&0x3f)<<6
                       | (static_cast<rune_t>(b2)&0x3f);
            cp.width = 3;
            return cp;
        }

        b3 = (uint8_t) str[3];
        if (!(b3 >= 0x80 && b3 <= 0xbf))
            return cp;

        cp.value = (static_cast<rune_t>(s0)&0x07)<<18
                   | (static_cast<rune_t>(b1)&0x3f)<<12
                   | (static_cast<rune_t>(b2)&0x3f)<<6
                   | (static_cast<rune_t>(b3)&0x3f);
        cp.width = 4;

        return cp;
    }

}