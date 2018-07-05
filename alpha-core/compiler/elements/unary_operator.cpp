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

#include "program.h"
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

    bool unary_operator::on_is_constant() const {
        return _rhs != nullptr && _rhs->is_constant();
    }

    // XXX: this requires lots of future love
    compiler::type* unary_operator::on_infer_type(const compiler::program* program) {
        switch (operator_type()) {
            case operator_type_t::negate:
            case operator_type_t::binary_not: {
                return program->find_type_up("u64");
            }
            case operator_type_t::logical_not: {
                return program->find_type_up("bool");
            }
            default:
                return nullptr;
        }
    }

};