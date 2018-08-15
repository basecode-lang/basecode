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

#include "label.h"
#include "statement.h"

namespace basecode::compiler {

    statement::statement(
            compiler::module* module,
            block* parent_scope,
            element* expr) : element(module, parent_scope, element_type_t::statement),
                             _expression(expr) {
    }

    bool statement::on_emit(
            common::result& r,
            emit_context_t& context) {
        if (_expression == nullptr)
            return true;

        // XXX: need to loop over labels and add them to the assembler here

        return _expression->emit(r, context);
    }

    element* statement::expression() {
        return _expression;
    }

    label_list_t& statement::labels() {
        return _labels;
    }

    void statement::on_owned_elements(element_list_t& list) {
        if (_expression != nullptr)
            list.emplace_back(_expression);

        for (auto element : _labels)
            list.emplace_back(element);
    }

};