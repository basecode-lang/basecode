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

//    bool assignment::on_emit(
//            compiler::session& session,
//            compiler::emit_context_t& context,
//            compiler::emit_result_t& result) {
//        for (auto expr : _expressions) {
//            auto success = expr->emit(session, context, result);
//            if (!success)
//                return false;
//        }
//        return true;
//    }

    element_list_t& assignment::expressions() {
        return _expressions;
    }

    void assignment::on_owned_elements(element_list_t& list) {
        for (auto x : _expressions)
            list.emplace_back(x);
    }

};