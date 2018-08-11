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
#include "return_element.h"

namespace basecode::compiler {

    return_element::return_element(
        block* parent_scope) : element(parent_scope, element_type_t::return_e) {
    }

    bool return_element::on_emit(
            common::result& r,
            emit_context_t& context) {
        auto instruction_block = context.assembler->current_block();
        if (!_expressions.empty()) {
            vm::register_t target_reg;
            if (!context.assembler->allocate_reg(target_reg)) {
            }
            context.assembler->push_target_register(target_reg);
            // XXX: temporarily, only the first return value
            _expressions.front()->emit(r, context);
            instruction_block->store_from_reg(
                vm::register_t::fp(),
                target_reg,
                8);
            context.assembler->pop_target_register();
            context.assembler->free_reg(target_reg);
        }
        instruction_block->rts();
        return true;
    }

    element_list_t& return_element::expressions() {
        return _expressions;
    }

    void return_element::on_owned_elements(element_list_t& list) {
        for (auto element : _expressions)
            list.emplace_back(element);
    }

};