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
            element* parent,
            operator_type_t type,
            element* rhs) : operator_base(parent, element_type_t::unary_operator, type),
                            _rhs(rhs) {
    }

    element* unary_operator::rhs() {
        return _rhs;
    }

};