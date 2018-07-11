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

#include "alias.h"

namespace basecode::compiler {

    alias::alias(
        element* parent,
        element* expr) : element(parent, element_type_t::alias_type),
                         _expression(expr) {
    }

    element* alias::expression() {
        return _expression;
    }

};