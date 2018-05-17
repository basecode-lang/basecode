#pragma once

#include <cstdint>

namespace basecode {

    inline bool is_platform_little_endian() {
        int n = 1;
        return (*(char*)&n) == 1;
    }

    inline uint8_t get_upper_nybble(uint8_t value) {
        return static_cast<uint8_t>((value & 0xf0) >> 4);
    }

    inline uint8_t get_lower_nybble(uint8_t value) {
        return static_cast<uint8_t>(value & 0x0f);
    }

    inline uint16_t endian_swap_word(uint16_t value) {
        return (value >> 8) | (value << 8);
    }

    inline uint32_t endian_swap_dword(uint32_t value) {
        return ((value >> 24) & 0xff)
               |  ((value << 8) & 0xff0000)
               |  ((value >> 8) & 0xff00)
               |  ((value << 24) & 0xff000000);
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

};