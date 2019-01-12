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
#include <vm/instruction_block.h>
#include "defer_element.h"

namespace basecode::compiler {

    defer_element::defer_element(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* expression) : element(module, parent_scope, element_type_t::defer),
                                             _expression(expression) {
    }

//    bool defer_element::on_emit(
//            compiler::session& session,
//            compiler::emit_context_t& context,
//            compiler::emit_result_t& result) {
//        if (_expression == nullptr)
//            return false;
//        return _expression->emit(session, context, result);
//    }

    compiler::element* defer_element::expression() {
        return _expression;
    }

    void defer_element::on_owned_elements(element_list_t& list) {
        if (_expression != nullptr)
            list.emplace_back(_expression);
    }

};