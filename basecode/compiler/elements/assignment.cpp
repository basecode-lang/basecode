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
#include "assignment.h"

namespace basecode::compiler {

    assignment::assignment(
            compiler::module* module,
            compiler::block* parent_scope) : element(module, parent_scope, element_type_t::assignment) {
    }

    compiler::element* assignment::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        auto copy = session.builder().make_assignment(new_scope);
        copy->_expressions = compiler::clone(session, new_scope, _expressions);
        return copy;
    }

    element_list_t& assignment::expressions() {
        return _expressions;
    }

    void assignment::on_owned_elements(element_list_t& list) {
        for (auto x : _expressions)
            list.emplace_back(x);
    }

}