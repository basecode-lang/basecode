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
#include "block.h"
#include "return_element.h"

namespace basecode::compiler {

    return_element::return_element(
        compiler::module* module,
        block* parent_scope) : element(module, parent_scope, element_type_t::return_e) {
    }

    bool return_element::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();
        if (!_expressions.empty()) {
            variable_handle_t expr_var {};
            if (!session.variable(_expressions.front(), expr_var))
                return false;
            expr_var->read();

            block->comment(
                "return slot",
                vm::comment_location_t::after_instruction);
            block->store(
                vm::instruction_operand_t::fp(),
                expr_var->emit_result().operands.back(),
                vm::instruction_operand_t(
                    static_cast<uint64_t>(16 /*frame.offsets().return_slot*/),
                    vm::op_sizes::byte));
            block->move(
                vm::instruction_operand_t::sp(),
                vm::instruction_operand_t::fp());
            block->pop(vm::instruction_operand_t::fp());
            block->rts();
        }
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