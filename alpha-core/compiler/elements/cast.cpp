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
#include "cast.h"
#include "symbol_element.h"

namespace basecode::compiler {

    cast::cast(
            compiler::module* module,
            block* parent_scope,
            compiler::type* type,
            element* expr) : element(module, parent_scope, element_type_t::cast),
                             _expression(expr),
                             _type(type) {
    }

    bool cast::on_emit(
            common::result& r,
            emit_context_t& context) {
        if (_expression == nullptr)
            return true;
        auto instruction_block = context.assembler->current_block();
        instruction_block->current_entry()->comment(
            fmt::format("XXX: cast<{}> not yet implemented", _type->symbol()->name()),
            context.indent);
        return _expression->emit(r, context);
    }

    element* cast::expression() {
        return _expression;
    }

    compiler::type* cast::type() {
        return _type;
    }

    void cast::on_owned_elements(element_list_t& list) {
        if (_expression != nullptr)
            list.emplace_back(_expression);
    }

    compiler::type* cast::on_infer_type(const compiler::program* program) {
        return _type;
    }

};