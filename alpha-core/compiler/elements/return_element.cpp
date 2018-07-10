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

#include <vm/instruction_block.h>
#include "return_element.h"

namespace basecode::compiler {

    return_element::return_element(
        element* parent) : element(parent, element_type_t::return_e) {
    }

    bool return_element::on_emit(
            common::result& r,
            vm::assembler& assembler,
            emit_context_t& context) {
        auto instruction_block = assembler.current_block();

        for (auto expr : _expressions) {
            auto target_reg = instruction_block->allocate_ireg();
            instruction_block->push_target_register(target_reg);
            expr->emit(r, assembler, context);
            instruction_block->pop_target_register();
            instruction_block->push_u32(target_reg);
            instruction_block->free_ireg(target_reg);
        }

        instruction_block->rts();

        return true;
    }

    element_list_t& return_element::expressions() {
        return _expressions;
    }

};