// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#include "type.h"
#include "field.h"

namespace basecode::compiler {

    type::type() {
    }

    type::~type() {
        for (auto field : _fields)
            delete field.second;
        _fields.clear();
    }

    void type::add_field(
            const std::string& name,
            compiler::type* type,
            compiler::initializer* initializer) {
        auto field = new compiler::field(name, type, initializer);
        field->type(type);
        _fields.insert(std::make_pair(name, field));
    }

    bool type::remove_field(const std::string& name) {
        return _fields.erase(name) > 0;
    }

    compiler::field* type::find_field(const std::string& name) {
        auto it = _fields.find(name);
        if (it != _fields.end())
            return it->second;
        return nullptr;
    }

};