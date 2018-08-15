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

#include <vm/instruction_block.h>
#include "type.h"
#include "transmute.h"
#include "symbol_element.h"

namespace basecode::compiler {

    transmute::transmute(
            compiler::module* module,
            block* parent_scope,
            compiler::type* type,
            element* expr) : element(module, parent_scope, element_type_t::cast),
                             _expression(expr),
                             _type(type) {
    }

    bool transmute::on_emit(
            common::result& r,
            emit_context_t& context) {
        if (_expression == nullptr)
            return true;
        auto instruction_block = context.assembler->current_block();
        instruction_block->current_entry()->comment(
            fmt::format("XXX: transmute<{}> not yet implemented", _type->symbol()->name()),
            context.indent);
        return _expression->emit(r, context);
    }

    element* transmute::expression() {
        return _expression;
    }

    compiler::type* transmute::type() {
        return _type;
    }

    void transmute::on_owned_elements(element_list_t& list) {
        if (_expression != nullptr)
            list.emplace_back(_expression);
    }

    compiler::type* transmute::on_infer_type(const compiler::program* program) {
        return _type;
    }

};