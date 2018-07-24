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
#include "module_reference.h"

namespace basecode::compiler {

    module_reference::module_reference(
            compiler::block* parent_scope,
            compiler::element* expr) : compiler::element(parent_scope, element_type_t::module_reference),
                                       _expression(expr) {
    }

    compiler::module* module_reference::module() {
        return _module;
    }

    bool module_reference::on_is_constant() const {
        return true;
    }

    compiler::element* module_reference::expression() {
        return _expression;
    }

    void module_reference::module(compiler::module* value) {
        _module = value;
    }

    void module_reference::on_owned_elements(element_list_t& list) {
        if (_expression != nullptr)
            list.emplace_back(_expression);
    }

    compiler::type* module_reference::on_infer_type(const compiler::program* program) {
        return program->find_type({.name = "module"});
    }

};

