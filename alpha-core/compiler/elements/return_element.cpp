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
            vm::i_registers_t target_reg;
            if (!instruction_block->allocate_reg(target_reg)) {
            }
            instruction_block->push_target_register(target_reg);
            // XXX: temporarily, only the first return value
            _expressions.front()->emit(r, context);
            instruction_block->store_from_ireg_u64(
                vm::i_registers_t::fp,
                target_reg,
                8);
            instruction_block->pop_target_register();
            instruction_block->free_reg(target_reg);
        }
        instruction_block->rts();
        return true;
    }

    element_list_t& return_element::expressions() {
        return _expressions;
    }

};