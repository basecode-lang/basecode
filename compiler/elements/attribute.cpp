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

#include "attribute.h"

namespace basecode::compiler {

    attribute::attribute(
            element* parent,
            const std::string& name,
            element* expr) : element(parent, element_type_t::attribute),
                             _name(name),
                             _expr(expr) {
    }

    element* attribute::expression() {
        return _expr;
    }

    std::string attribute::name() const {
        return _name;
    }

};