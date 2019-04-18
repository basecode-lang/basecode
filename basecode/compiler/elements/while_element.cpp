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
#include "block.h"
#include "while_element.h"
#include "binary_operator.h"

namespace basecode::compiler {

    while_element::while_element(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::binary_operator* predicate,
            compiler::block* body) : element(module, parent_scope, element_type_t::while_e),
                                     _body(body),
                                     _predicate(predicate) {
    }

    compiler::block* while_element::body() {
        return _body;
    }

    bool while_element::on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) {
        _predicate = fold_result.element;
        return true;
    }

    compiler::element* while_element::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        return session.builder().make_while(
            new_scope,
            _predicate->clone<compiler::binary_operator>(session, new_scope),
            _body != nullptr ? _body->clone<compiler::block>(session, new_scope) : nullptr);
    }

    compiler::element* while_element::predicate() {
        return _predicate;
    }

    void while_element::predicate(compiler::element* value) {
        _predicate = value;
    }

    void while_element::on_owned_elements(element_list_t& list) {
        if (_body != nullptr)
            list.emplace_back(_body);

        if (_predicate != nullptr)
            list.emplace_back(_predicate);
    }

}