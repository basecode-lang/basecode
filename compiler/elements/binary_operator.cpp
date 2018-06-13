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
        operator_type_t type,
        element* lhs,
        element* rhs) : operator_base(type),
                        _lhs(lhs),
                        _rhs(rhs) {

    }

    binary_operator::~binary_operator() {
    }

    element* binary_operator::lhs() {
        return _lhs;
    }

    element* binary_operator::rhs() {
        return _rhs;
    }

};