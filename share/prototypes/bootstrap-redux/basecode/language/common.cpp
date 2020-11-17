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

#include <basecode/numbers/bytes.h>
#include "common.h"

namespace basecode::language {

    void apply_narrowed_value(
            number_token_t& token,
            number_size_t size,
            double value) {
        token.size = size;
        switch (size) {
            case number_size_t::byte:
            case number_size_t::word:
                break;
            case number_size_t::dword:
                token.value.f32 = (float)value;
                if (!token.is_signed)
                    token.is_signed = token.value.f32 < 0;
                break;
            case number_size_t::qword:
                token.value.f64 = value;
                if (!token.is_signed)
                    token.is_signed = token.value.f64 < 0;
                break;
        }
    }

    void apply_narrowed_value(
            number_token_t& token,
            number_size_t size,
            int64_t value,
            bool check_sign_bit) {
        token.size = size;
        switch (size) {
            case number_size_t::byte:
                token.value.u8 = static_cast<uint8_t>(value);
                if (check_sign_bit)
                    token.is_signed = numbers::is_sign_bit_set(token.value.u8);
                break;
            case number_size_t::word:
                token.value.u16 = static_cast<uint16_t>(value);
                if (check_sign_bit)
                    token.is_signed = numbers::is_sign_bit_set(token.value.u16);
                break;
            case number_size_t::dword:
                token.value.u32 = static_cast<uint32_t>(value);
                if (check_sign_bit)
                    token.is_signed = numbers::is_sign_bit_set(token.value.u32);
                break;
            case number_size_t::qword:
                token.value.u64 = static_cast<uint64_t>(value);
                if (check_sign_bit)
                    token.is_signed = numbers::is_sign_bit_set(token.value.u64);
                break;
        }
    }

    std::optional<number_size_t> narrow_type(double value) {
        if (value < -3.4e+38 || value > 3.4e+38)
            return number_size_t::qword;
        else if (value >= -3.4e+38 && value <= 3.4e+38)
            return number_size_t::dword;
        else
            return {};
    }

    std::optional<number_size_t> narrow_type(int64_t value) {
        if (value == 0)
            return number_size_t::byte;

        auto unsigned_value = static_cast<uint64_t>(value);
        uint64_t mask = 0b0000000000000000000000000000000000000000000000000000000000000001;

        uint32_t max_bit_pos = 0;
        for (size_t i = 0; i < 64; i++) {
            if ((unsigned_value & mask) == mask) {
                max_bit_pos = i;
            }
            mask <<= 1;
        }

        max_bit_pos = std::max<uint32_t>(
            8,
            numbers::next_power_of_two(max_bit_pos));
        auto size_in_bytes = max_bit_pos / 8;

        switch (size_in_bytes) {
            case 1: return number_size_t::byte;
            case 2: return number_size_t::word;
            case 4: return number_size_t::dword;
            case 8: return number_size_t::qword;
            default:
                break;
        }

        return {};
    }

    std::string_view number_size_to_name(number_size_t size) {
        switch (size) {
            case number_size_t::byte:  return "byte"sv;
            case number_size_t::word:  return "word"sv;
            case number_size_t::dword: return "dword"sv;
            case number_size_t::qword: return "qword"sv;
        }
    }

    std::string_view number_type_to_name(number_type_t type) {
        switch (type) {
            case number_type_t::none:           return "none"sv;
            case number_type_t::integer:        return "integer"sv;
            case number_type_t::arbitrary:      return "arbitrary"sv;
            case number_type_t::floating_point: return "floating_point"sv;
        }
    }

}