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

#include "program.h"
#include "namespace_element.h"

namespace basecode::compiler {

    namespace_element::namespace_element(
        element* parent,
        element* expr) : element(parent, element_type_t::namespace_e),
                         _expression(expr) {
    }

    element* namespace_element::expression() {
        return _expression;
    }

    bool namespace_element::on_is_constant() const {
        return true;
    }

    compiler::type* namespace_element::on_infer_type(const compiler::program* program) {
        return program->find_type_up("namespace");
    }

};