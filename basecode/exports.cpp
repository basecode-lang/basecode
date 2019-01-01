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

#include <cstdio>
#include <cstdarg>
#include <cstdint>
#include <fmt/format.h>

extern "C" {
    void fmt_print(const char* fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
    }

    struct bc_string {
        uint32_t length;
        uint32_t capacity;
        uint8_t* data;
    };

    struct bc_vector3 {
        float x, y, z;
    };

    struct bc_player {
        bc_string name;
        uint8_t lives;
        uint16_t hp;
    };

    struct bc_entity {
        bc_string name;
        bc_vector3 pos;
        bc_player* data;
    };

    void str_test_ptr(struct bc_string* str) {
        fmt::print("str.length = {}\n", str->length);
        fmt::print("str.capacity = {}\n", str->capacity);
        fmt::print("str.data = {}\n", str->data);
    }

    void str_test_cpy(struct bc_string str) {
        fmt::print("str_test_cpy does *not* work!\n");
//        fmt::print("str.length = {}\n", str.length);
//        fmt::print("str.capacity = {}\n", str.capacity);
//        fmt::print("str.data = {}\n", str.data);
    }

    void complex_test_ptr(struct bc_entity* e) {
        fmt::print("e.name = {}\n", e->name.data);
        fmt::print("e.data->hp = {}\n", e->data->hp);
        fmt::print("e.data->lives = {}\n", e->data->lives);
        fmt::print("e.data->name = {}\n", e->data->name.data);
    }

};
