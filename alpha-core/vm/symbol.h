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

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <common/id_pool.h>

namespace basecode::vm {

    class symbol;

    using symbol_list_t = std::vector<symbol*>;

    enum class symbol_type_t {
        unknown,
        u8,
        u16,
        u32,
        u64,
        f32,
        f64,
        bytes
    };

    static inline std::unordered_map<symbol_type_t, std::string> s_symbol_type_names = {
        {symbol_type_t::unknown, "unknown"},
        {symbol_type_t::u8,      "u8"},
        {symbol_type_t::u16,     "u16"},
        {symbol_type_t::u32,     "u32"},
        {symbol_type_t::u64,     "u64"},
        {symbol_type_t::f32,     "f32"},
        {symbol_type_t::f64,     "f64"},
        {symbol_type_t::bytes,   "bytes"},
    };

    static inline std::string symbol_type_name(symbol_type_t type) {
        auto it = s_symbol_type_names.find(type);
        if (it == s_symbol_type_names.end())
            return "unknown";
        return it->second;
    }

    static inline size_t size_of_symbol_type(symbol_type_t type) {
        switch (type) {
            case symbol_type_t::u8:    return 1;
            case symbol_type_t::u16:   return 2;
            case symbol_type_t::u32:   return 4;
            case symbol_type_t::u64:   return 8;
            case symbol_type_t::f32:   return 4;
            case symbol_type_t::f64:   return 8;
            default:
                return 0;
        }
    }

    static inline symbol_type_t float_symbol_type_for_size(size_t size) {
        switch (size) {
            case 4:
                return symbol_type_t::f32;
            case 8:
                return symbol_type_t::f64;
            default:
                return symbol_type_t::unknown;
        }
    }

    static inline symbol_type_t integer_symbol_type_for_size(size_t size) {
        switch (size) {
            case 1:
                return symbol_type_t::u8;
            case 2:
                return symbol_type_t::u16;
            case 4:
                return symbol_type_t::u32;
            case 8:
                return symbol_type_t::u64;
            default:
                return symbol_type_t::unknown;
        }
    }

    class symbol {
    public:
        symbol(
            const std::string& name,
            symbol_type_t type,
            uint64_t offset,
            size_t size = 0);

        void value(void* v);

        size_t size() const;

        void value(double v);

        void value(uint64_t v);

        uint64_t offset() const;

        std::string name() const;

        symbol_type_t type() const;

        common::id_t pending_address_from_id() const;

        void pending_address_from_id(common::id_t value);

    private:
        size_t _size;
        uint64_t _offset;
        std::string _name;
        symbol_type_t _type;
        common::id_t _pending_address_from_id = 0;

        union {
            double float_value;
            uint64_t int_value;
            void* byte_array_value;
        } _value;
    };

};

