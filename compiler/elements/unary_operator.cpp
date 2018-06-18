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

#include "unary_operator.h"

namespace basecode::compiler {

    unary_operator::unary_operator(
            operator_type_t type,
            expression* rhs) : operator_base(type),
                               _rhs(rhs) {
    }

    unary_operator::~unary_operator() {
    }

    expression* unary_operator::rhs() {
        return _rhs;
    }

};