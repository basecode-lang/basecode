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
        binary_operator* assignment) : expression(dynamic_cast<element*>(assignment)) {
    }

    initializer::~initializer() {
    }

    binary_operator* initializer::assignment() {
        return _assignment;
    }

};