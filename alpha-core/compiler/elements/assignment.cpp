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

#include "assignment.h"

namespace basecode::compiler {

    assignment::assignment(
            compiler::module* module,
            compiler::block* parent_scope) : element(module, parent_scope, element_type_t::assignment) {
    }

    element_list_t& assignment::expressions() {
        return _expressions;
    }

    bool assignment::on_emit(compiler::session& session) {
        for (auto expr : _expressions) {
            auto success = expr->emit(session);
            if (!success)
                return false;
        }
        return true;
    }

    void assignment::on_owned_elements(element_list_t& list) {
        for (auto x : _expressions)
            list.emplace_back(x);
    }

};