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

#include <cstdint>
#include <climits>

namespace basecode::numbers {

    inline uint64_t bit(uint64_t n) {
        return static_cast<uint64_t>(1) << n;
    }

    inline uint8_t to_hex(uint8_t x) {
        return x + (x > 9 ? ('A' - 10) : '0');
    }

    template <typename T>
    inline T twos_complement(T value) {
        return ~value + 1;
    }

    inline uint8_t from_hex(uint8_t ch) {
        if (ch <= '9' && ch >= '0')
            ch -= '0';
        else if (ch <= 'f' && ch >= 'a')
            ch -= 'a' - 10;
        else if (ch <= 'F' && ch >= 'A')
            ch -= 'A' - 10;
        else
            ch = 0;
        return ch;
    }

    template <typename T>
    inline bool is_sign_bit_set(T value) {
        return (value & (static_cast<T>(1) << ((sizeof(T) * CHAR_BIT) - 1))) != 0;
    }

    inline bool is_power_of_two(int64_t x) {
        if (x <= 0)
            return false;
        return !(x & (x-1));
    }

    inline bool is_platform_little_endian() {
        int n = 1;
        return (*(char*)&n) == 1;
    }

    inline uint64_t rotl(uint64_t n, uint8_t c) {
        const unsigned int mask = (CHAR_BIT * sizeof(n) - 1);
        c &= mask;
        return (n << c) | (n >> ((-c) & mask));
    }

    inline uint64_t rotr(uint64_t n, uint8_t c) {
        const unsigned int mask = (CHAR_BIT * sizeof(n) - 1);
        c &= mask;
        return (n >> c) | (n << ((-c) & mask));
    }

    inline uint8_t get_upper_nybble(uint8_t value) {
        return static_cast<uint8_t>((value & 0xf0) >> 4);
    }

    inline uint8_t get_lower_nybble(uint8_t value) {
        return static_cast<uint8_t>(value & 0x0f);
    }

    inline uint32_t next_power_of_two(uint32_t n) {
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n++;
        return n;
    }

    inline uint64_t next_power_of_two(uint64_t n) {
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n |= n >> 32;
        n++;
        return n;
    }

    inline uint64_t power(uint64_t x, uint64_t n) {
        uint64_t pow = 1;
        while (n) {
            if (n & 1)
                pow *= x;
            n = n >> 1;
            x = x * x;
        }
        return pow;
    }

    inline uint16_t endian_swap_word(uint16_t value) {
        return (value >> 8) | (value << 8);
    }

    inline uint32_t previous_power_of_two(uint32_t n) {
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        return n - (n >> 1);
    }

    inline uint64_t previous_power_of_two(uint64_t n) {
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n |= n >> 32;
        return n - (n >> 1);
    }

    inline uint32_t endian_swap_dword(uint32_t value) {
        return ((value >> 24) & 0xff)
               |  ((value << 8) & 0xff0000)
               |  ((value >> 8) & 0xff00)
               |  ((value << 24) & 0xff000000);
    }

    inline uint64_t endian_swap_qword(uint64_t value) {
        return ((value & 0x00000000000000ffu) << 56) |
               ((value & 0x000000000000ff00u) << 40) |
               ((value & 0x0000000000ff0000u) << 24) |
               ((value & 0x00000000ff000000u) << 8)  |
               ((value & 0x000000ff00000000u) >> 8)  |
               ((value & 0x0000ff0000000000u) >> 24) |
               ((value & 0x00ff000000000000u) >> 40) |
               ((value & 0xff00000000000000u) >> 56);
    }

    inline uint64_t align(uint64_t size, uint64_t align) {
        if (align > 0) {
            auto result = size + align - 1;
            return result - result % align;
        }
        return size;
    }

    inline uint64_t sign_extend(int64_t value, uint32_t bits) {
        auto shift = sizeof(uint64_t) * CHAR_BIT - bits;
        auto result = (value << shift) >> shift;
        return static_cast<uint64_t>(result);
    }

    inline uint8_t set_lower_nybble(uint8_t original, uint8_t value) {
        uint8_t res = original;
        res &= 0xF0;
        res |= (value & 0x0F);
        return res;
    }

    inline uint8_t set_upper_nybble(uint8_t original, uint8_t value) {
        uint8_t res = original;
        res &= 0x0F;
        res |= ((value << 4) & 0xF0);
        return res;
    }

}