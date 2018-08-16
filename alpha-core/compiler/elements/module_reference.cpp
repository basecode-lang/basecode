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

#include <compiler/session.h>
#include "program.h"
#include "module_reference.h"

namespace basecode::compiler {

    module_reference::module_reference(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* expr) : compiler::element(module, parent_scope, element_type_t::module_reference),
                                       _expression(expr) {
    }

    bool module_reference::on_is_constant() const {
        return true;
    }

    compiler::module* module_reference::reference() {
        return _reference;
    }

    compiler::element* module_reference::expression() {
        return _expression;
    }

    void module_reference::reference(compiler::module* value) {
        _reference = value;
    }

    void module_reference::on_owned_elements(element_list_t& list) {
        if (_expression != nullptr)
            list.emplace_back(_expression);
    }

    compiler::type* module_reference::on_infer_type(const compiler::session& session) {
        return session.scope_manager().find_type({.name = "module"});
    }

};

