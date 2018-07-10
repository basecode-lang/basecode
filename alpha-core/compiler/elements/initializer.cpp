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

#include <vm/instruction_block.h>
#include "initializer.h"
#include "procedure_type.h"
#include "binary_operator.h"

namespace basecode::compiler {

    initializer::initializer(
            element* parent,
            element* expr) : element(parent, element_type_t::initializer),
                             _expr(expr) {
    }

    bool initializer::on_emit(
            common::result& r,
            vm::assembler& assembler,
            emit_context_t& context) {
        return true;
    }

    element* initializer::expression() {
        return _expr;
    }

    bool initializer::on_as_bool(bool& value) const {
        if (_expr == nullptr)
            return false;
        return _expr->as_bool(value);
    }

    bool initializer::on_as_float(double& value) const {
        if (_expr == nullptr)
            return false;
        return _expr->as_float(value);
    }

    bool initializer::on_as_integer(uint64_t& value) const {
        if (_expr == nullptr)
            return false;
        return _expr->as_integer(value);
    }

    compiler::procedure_type* initializer::procedure_type() {
        if (_expr == nullptr || _expr->element_type() != element_type_t::proc_type)
            return nullptr;
        return dynamic_cast<compiler::procedure_type*>(_expr);
    }

    bool initializer::on_as_string(std::string& value) const {
        if (_expr == nullptr)
            return false;
        return _expr->as_string(value);
    }

};