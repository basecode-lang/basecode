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

};