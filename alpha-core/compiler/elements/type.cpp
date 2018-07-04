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

    type::type(
        element* parent,
        element_type_t type,
        const std::string& name) : element(parent, type),
                                   _name(name) {
    }

    std::string type::name() const {
        return _name;
    }

    size_t type::size_in_bytes() const {
        return _size_in_bytes;
    }

    void type::size_in_bytes(size_t value) {
        _size_in_bytes = value;
    }

    bool type::initialize(common::result& r) {
        return on_initialize(r);
    }

    void type::name(const std::string& value) {
        _name = value;
    }

    bool type::on_initialize(common::result& r) {
        return true;
    }

};