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
#include "float_literal.h"
#include "string_literal.h"
#include "integer_literal.h"
#include "boolean_literal.h"

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

    std::string attribute::as_string() const {
        switch (_expr->element_type()) {
            case element_type_t::float_literal:
                return std::to_string(dynamic_cast<compiler::float_literal*>(_expr)->value());
            case element_type_t::string_literal:
                return dynamic_cast<compiler::string_literal*>(_expr)->value();
            case element_type_t::boolean_literal:
                return std::to_string(dynamic_cast<compiler::boolean_literal*>(_expr)->value());
            case element_type_t::integer_literal:
                return std::to_string(dynamic_cast<compiler::integer_literal*>(_expr)->value());
            default:
                break;
        }
        return "";
    }

};