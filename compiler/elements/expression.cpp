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

#include "expression.h"

namespace basecode::compiler {

    expression::expression(
            element* parent,
            element* root) : element(parent),
                             _root(root) {
    }

    expression::~expression() {
    }

};