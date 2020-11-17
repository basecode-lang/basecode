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
#include "segment.h"

namespace basecode::vm {

    segment::segment(
        const std::string& name,
        segment_type_t type) : _name(name),
                               _type(type) {
    }

    vm::symbol* segment::symbol(
            const std::string& name,
            symbol_type_t type,
            size_t size) {
        auto type_size = size == 0 ? size_of_symbol_type(type) : size;

        _symbols.insert(std::make_pair(
            name,
            vm::symbol(name, type, _offset, type_size)));

        _offset += type_size;

        return symbol(name);
    }

    size_t segment::size() const {
        return _offset;
    }

    bool segment::initialized() const {
        return _initialized;
    }

    std::string segment::name() const {
        return _name;
    }

    segment_type_t segment::type() const {
        return _type;
    }

    void segment::initialized(bool value) {
        _initialized = value;
    }

    symbol_list_t segment::symbols() const {
        symbol_list_t list {};
        for (const auto& it : _symbols)
            list.push_back(const_cast<vm::symbol*>(&it.second));
        return list;
    }

    vm::symbol* segment::symbol(const std::string& name) {
        auto it = _symbols.find(name);
        if (it == _symbols.end())
            return nullptr;
        return &it->second;
    }

};