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
#include <compiler/element_builder.h>
#include "expression.h"

namespace basecode::compiler {

    expression::expression(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* root) : element(module, parent_scope, element_type_t::expression),
                                       _root(root) {
    }

    bool expression::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        if (_root == nullptr)
            return false;
        return _root->infer_type(session, result);
    }

    bool expression::on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) {
        _root = fold_result.element;
        return true;
    }

    compiler::element* expression::root() {
        return _root;
    }

    compiler::element* expression::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        return session.builder().make_expression(
            new_scope,
            _root != nullptr ? _root->clone<compiler::element>(session, new_scope) : nullptr);
    }

    bool expression::on_is_constant() const {
        if (_root == nullptr)
            return false;
        return _root->is_constant();
    }

    bool expression::on_as_bool(bool& value) const {
        if (_root == nullptr)
            return false;
        return _root->as_bool(value);
    }

    void expression::root(compiler::element* value) {
        _root = value;
    }

    bool expression::on_as_float(double& value) const {
        if (_root == nullptr)
            return false;
        return _root->as_float(value);
    }

    bool expression::on_as_integer(uint64_t& value) const {
        if (_root == nullptr)
            return false;
        return _root->as_integer(value);
    }

    void expression::on_owned_elements(element_list_t& list) {
        if (_root != nullptr)
            list.emplace_back(_root);
    }

}