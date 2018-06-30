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

#include "initializer.h"
#include "procedure_type.h"
#include "binary_operator.h"

namespace basecode::compiler {

    initializer::initializer(
            element* parent,
            element* expr) : element(parent, element_type_t::initializer),
                             _expr(expr) {
    }

    element* initializer::expression() {
        return _expr;
    }

    compiler::procedure_type* initializer::procedure_type() {
        if (_expr == nullptr || _expr->element_type() != element_type_t::proc_type)
            return nullptr;
        return dynamic_cast<compiler::procedure_type*>(_expr);
    }

};