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
            block* parent_scope,
            const std::string& name,
            element* expr) : element(parent_scope, element_type_t::namespace_e),
                             _name(name),
                             _expression(expr) {
    }

    bool namespace_element::on_emit(
            common::result& r,
            emit_context_t& context) {
        if (_expression == nullptr)
            return true;
        return _expression->emit(r, context);
    }

    element* namespace_element::expression() {
        return _expression;
    }

    std::string namespace_element::name() const {
        return _name;
    }

    bool namespace_element::on_is_constant() const {
        return true;
    }

    void namespace_element::name(const std::string& value) {
        _name = value;
    }

    compiler::type* namespace_element::on_infer_type(const compiler::program* program) {
        return program->find_type_up("namespace");
    }

};