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
#include "argument_pair.h"

namespace basecode::compiler {

    argument_pair::argument_pair(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* lhs,
            compiler::element* rhs) : element(module, parent_scope, element_type_t::argument_pair),
                                      _lhs(lhs),
                                      _rhs(rhs) {
    }

    bool argument_pair::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        return _rhs->infer_type(session, result);
    }

    compiler::element* argument_pair::lhs() {
        return _lhs;
    }

    compiler::element* argument_pair::rhs() {
        return _rhs;
    }

    bool argument_pair::on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) {
        _rhs = fold_result.element;
        return true;
    }

    bool argument_pair::on_is_constant() const {
        return true;
    }

    void argument_pair::rhs(compiler::element* value) {
        _rhs = value;
    }

    void argument_pair::on_owned_elements(element_list_t& list) {
        list.emplace_back(_lhs);
        list.emplace_back(_rhs);
    }

    compiler::element* argument_pair::on_clone(compiler::session& session) {
        return session.builder().make_argument_pair(
            parent_scope(),
            _lhs->clone(session),
            _rhs->clone(session));
    }

}