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
#include "spread_operator.h"

namespace basecode::compiler {

    spread_operator::spread_operator(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* expr) : compiler::element(module, parent_scope, element_type_t::spread_operator),
                                       _expr(expr) {
    }

    bool spread_operator::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        return _expr->infer_type(session, result);
    }

    compiler::element* spread_operator::expr() {
        return _expr;
    }

    compiler::element* spread_operator::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        return session.builder().make_spread_operator(
            new_scope,
            _expr->clone<compiler::element>(session, new_scope));
    }

    void spread_operator::on_owned_elements(element_list_t& list) {
        if (_expr != nullptr)
            list.emplace_back(_expr);
    }

}