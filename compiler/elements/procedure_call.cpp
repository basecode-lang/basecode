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

#include "procedure_call.h"

namespace basecode::compiler {

    procedure_call::procedure_call(
        element* parent,
        compiler::type* procedure_type,
        element* expr) : element(parent, element_type_t::proc_call),
                         _expression(expr),
                         _procedure_type(procedure_type) {
    }

    element* procedure_call::expression() {
        return _expression;
    }

    compiler::type* procedure_call::procedure_type() {
        return _procedure_type;
    }

};