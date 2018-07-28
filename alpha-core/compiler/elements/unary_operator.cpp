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
#include "program.h"
#include "unary_operator.h"

namespace basecode::compiler {

    unary_operator::unary_operator(
            block* parent_scope,
            operator_type_t type,
            element* rhs) : operator_base(parent_scope, element_type_t::unary_operator, type),
                            _rhs(rhs) {
    }

    bool unary_operator::on_emit(
            common::result& r,
            emit_context_t& context) {
        auto instruction_block = context.assembler->current_block();
        auto target_reg = instruction_block->current_target_register();
        vm::i_registers_t rhs_reg;
        if (!instruction_block->allocate_reg(rhs_reg)) {
            // XXX: error
        }
        instruction_block->push_target_register(rhs_reg);
        _rhs->emit(r, context);
        instruction_block->pop_target_register();

        switch (operator_type()) {
            case operator_type_t::negate: {
                instruction_block->neg_u64(target_reg->reg.i, rhs_reg);
                break;
            }
            case operator_type_t::binary_not: {
                instruction_block->not_u64(target_reg->reg.i, rhs_reg);
                break;
            }
            case operator_type_t::logical_not: {
                break;
            }
            default:
                break;
        }

        instruction_block->free_reg(rhs_reg);

        return true;
    }

    element* unary_operator::rhs() {
        return _rhs;
    }

    bool unary_operator::on_is_constant() const {
        return _rhs != nullptr && _rhs->is_constant();
    }

    void unary_operator::on_owned_elements(element_list_t& list) {
        if (_rhs != nullptr)
            list.emplace_back(_rhs);
    }

    // XXX: this requires lots of future love
    compiler::type* unary_operator::on_infer_type(const compiler::program* program) {
        switch (operator_type()) {
            case operator_type_t::negate:
            case operator_type_t::binary_not: {
                return program->find_type({"u64"});
            }
            case operator_type_t::logical_not: {
                return program->find_type({"bool"});
            }
            default:
                return nullptr;
        }
    }

};