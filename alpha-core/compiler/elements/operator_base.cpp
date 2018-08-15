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

#include "operator_base.h"

namespace basecode::compiler {

    operator_base::operator_base(
            compiler::module* module,
            block* parent_scope,
            element_type_t element_type,
            operator_type_t operator_type) : element(module, parent_scope, element_type),
                                             _operator_type(operator_type) {
    }

    operator_type_t operator_base::operator_type() const {
        return _operator_type;
    }

};