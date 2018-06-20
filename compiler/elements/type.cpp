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

    void type::name(const std::string& value) {
        _name = value;
    }

};