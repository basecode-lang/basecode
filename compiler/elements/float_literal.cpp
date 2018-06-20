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

#include "float_literal.h"

namespace basecode::compiler {

    float_literal::float_literal(
            element* parent,
            double value) : element(parent, element_type_t::float_literal),
                            _value(value) {
    }

    double float_literal::value() const {
        return _value;
    }

};