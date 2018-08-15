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

#include <vm/instruction_block.h>
#include "initializer.h"
#include "procedure_type.h"
#include "binary_operator.h"

namespace basecode::compiler {

    initializer::initializer(
            compiler::module* module,
            block* parent_scope,
            element* expr) : element(module, parent_scope, element_type_t::initializer),
                             _expr(expr) {
    }

    element* initializer::expression() {
        return _expr;
    }

    void initializer::expression(element* value) {
        _expr = value;
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

    bool initializer::on_emit(compiler::session& session) {
        if (_expr == nullptr)
            return true;

        if (_expr->element_type() == element_type_t::namespace_e)
            _expr->emit(session);

        return true;
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

    void initializer::on_owned_elements(element_list_t& list) {
        if (_expr != nullptr)
            list.emplace_back(_expr);
    }

    compiler::type* initializer::on_infer_type(const compiler::program* program) {
        if (_expr != nullptr)
            return _expr->infer_type(program);
        return nullptr;
    }

};