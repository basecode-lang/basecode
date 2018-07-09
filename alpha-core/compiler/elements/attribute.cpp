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

    bool attribute::on_as_bool(bool& value) const {
        if (_expr == nullptr)
            return false;
        return _expr->as_bool(value);
    }

    bool attribute::on_as_float(double& value) const {
        if (_expr == nullptr)
            return false;
        return _expr->as_float(value);
    }

    bool attribute::on_as_integer(uint64_t& value) const {
        if (_expr == nullptr)
            return false;
        return _expr->as_integer(value);
    }

    bool attribute::on_as_string(std::string& value) const {
        if (_expr == nullptr)
            return false;
        return _expr->as_string(value);
    }

};