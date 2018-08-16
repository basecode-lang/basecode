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
#include "expression.h"

namespace basecode::compiler {

    expression::expression(
            compiler::module* module,
            block* parent_scope,
            element* root) : element(module, parent_scope, element_type_t::expression),
                             _root(root) {
    }

    element* expression::root() {
        return _root;
    }

    bool expression::on_is_constant() const {
        if (_root == nullptr)
            return false;
        return _root->is_constant();
    }

    bool expression::on_emit(compiler::session& session) {
        if (_root == nullptr)
            return true;
        return _root->emit(session);
    }

    void expression::on_owned_elements(element_list_t& list) {
        if (_root != nullptr)
            list.emplace_back(_root);
    }

    compiler::type* expression::on_infer_type(const compiler::session& session) {
        if (_root == nullptr)
            return nullptr;
        return _root->infer_type(session);
    }

};