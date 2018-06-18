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

#include "binary_operator.h"

namespace basecode::compiler {

    binary_operator::binary_operator(
        element* parent,
        operator_type_t type,
        expression* lhs,
        expression* rhs) : operator_base(parent, type),
                           _lhs(lhs),
                           _rhs(rhs) {
    }

    binary_operator::~binary_operator() {
    }

    expression* binary_operator::lhs() {
        return _lhs;
    }

    expression* binary_operator::rhs() {
        return _rhs;
    }

};