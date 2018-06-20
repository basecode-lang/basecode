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

#include "boolean_literal.h"

namespace basecode::compiler {

    boolean_literal::boolean_literal(
            element* parent,
            bool value) : element(parent, element_type_t::boolean_literal),
                          _value(value) {
    }

    bool boolean_literal::value() const {
        return _value;
    }

};