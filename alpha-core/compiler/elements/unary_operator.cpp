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
#include "program.h"
#include "unary_operator.h"

namespace basecode::compiler {

    unary_operator::unary_operator(
            element* parent,
            operator_type_t type,
            element* rhs) : operator_base(parent, element_type_t::unary_operator, type),
                            _rhs(rhs) {
    }

    bool unary_operator::on_emit(
            common::result& r,
            vm::assembler& assembler) {
        auto instruction_block = assembler.current_block();
        switch (operator_type()) {
            case operator_type_t::negate: {
                // XXX: how to best handle the rhs here?
                auto src_reg = instruction_block->allocate_ireg();
                instruction_block->move_u64_to_ireg(src_reg, 0xc0fefe);

                auto dest_reg = instruction_block->allocate_ireg();
                instruction_block->neg_u64(dest_reg, src_reg);
                instruction_block->free_ireg(dest_reg);
                instruction_block->free_ireg(src_reg);
                break;
            }
            case operator_type_t::binary_not:
                break;
            case operator_type_t::logical_not:
                break;
            default:
                break;
        }
        return true;
    }

    element* unary_operator::rhs() {
        return _rhs;
    }

    bool unary_operator::on_is_constant() const {
        return _rhs != nullptr && _rhs->is_constant();
    }

    // XXX: this requires lots of future love
    compiler::type* unary_operator::on_infer_type(const compiler::program* program) {
        switch (operator_type()) {
            case operator_type_t::negate:
            case operator_type_t::binary_not: {
                return program->find_type_up("u64");
            }
            case operator_type_t::logical_not: {
                return program->find_type_up("bool");
            }
            default:
                return nullptr;
        }
    }

};