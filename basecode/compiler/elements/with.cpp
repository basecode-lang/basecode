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
#include "with.h"
#include "type.h"
#include "block.h"
#include "identifier.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    with::with(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* expr,
            compiler::block* body) : element(module, parent_scope, element_type_t::with),
                                     _body(body),
                                     _expr(expr) {
    }

    compiler::block* with::body() {
        return _body;
    }

    compiler::element* with::expr() {
        return _expr;
    }

    compiler::element* with::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        return session.builder().make_with(
            new_scope,
            _expr->clone<compiler::element>(session, new_scope),
            _body->clone<compiler::block>(session, new_scope));
    }

    void with::on_owned_elements(element_list_t& list) {
        if (_expr != nullptr)
            list.emplace_back(_expr);

        if (_body != nullptr)
            list.emplace_back(_body);
    }

}