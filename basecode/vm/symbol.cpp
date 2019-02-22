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

#include "symbol.h"

namespace basecode::vm {

    symbol::symbol(
            const std::string& name,
            symbol_type_t type,
            uint64_t offset,
            size_t size) : _size(size),
                           _offset(offset),
                           _name(name),
                           _type(type) {
    }

    void symbol::value(void* v) {
        _value.byte_array_value = v;
    }

    size_t symbol::size() const {
        return _size;
    }

    void symbol::value(double v) {
        _value.float_value = v;
    }

    void symbol::value(uint64_t v) {
        _value.int_value = v;
    }

    uint64_t symbol::offset() const {
        return _offset;
    }

    std::string symbol::name() const {
        return _name;
    }

    symbol_type_t symbol::type() const {
        return _type;
    }

};