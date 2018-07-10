// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#include "statement.h"

namespace basecode::compiler {

    statement::statement(
            element* parent,
            element* expr) : element(parent, element_type_t::statement),
                             _expression(expr) {
    }

    bool statement::on_emit(
            common::result& r,
            vm::assembler& assembler,
            emit_context_t& context) {
        if (_expression == nullptr)
            return true;

        // XXX: need to loop over labels and add them to the assembler here

        return _expression->emit(r, assembler, context);
    }

    element* statement::expression() {
        return _expression;
    }

    label_list_t& statement::labels() {
        return _labels;
    }

};