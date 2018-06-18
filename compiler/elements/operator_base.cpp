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

#include "operator_base.h"

namespace basecode::compiler {

    operator_base::operator_base(
            element* parent,
            operator_type_t type) : element(parent),
                                    _type(type) {
    }

    operator_base::~operator_base() {
    }

    operator_type_t operator_base::type() const {
        return _type;
    }

};