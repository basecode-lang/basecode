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

#include "element.h"
#include "program.h"
#include "identifier.h"
#include "binary_operator.h"

namespace basecode::compiler {

    binary_operator::binary_operator(
        element* parent,
        operator_type_t type,
        element* lhs,
        element* rhs) : operator_base(parent, element_type_t::binary_operator, type),
                        _lhs(lhs),
                        _rhs(rhs) {
    }

    bool binary_operator::on_emit(
            common::result& r,
            vm::assembler& assembler,
            const emit_context_t& context) {
        auto instruction_block = assembler.current_block();
        switch (operator_type()) {
            case operator_type_t::add: {
                auto result_reg = instruction_block->current_target_register();

                auto lhs_reg = instruction_block->allocate_ireg();
                instruction_block->push_target_register(lhs_reg);
                _lhs->emit(r, assembler, context);
                instruction_block->pop_target_register();

                auto rhs_reg = instruction_block->allocate_ireg();
                instruction_block->push_target_register(rhs_reg);
                _rhs->emit(r, assembler, context);
                instruction_block->pop_target_register();

                instruction_block->add_ireg_by_ireg_u64(
                    result_reg->reg.i,
                    lhs_reg,
                    rhs_reg);

                instruction_block->free_ireg(lhs_reg);
                instruction_block->free_ireg(rhs_reg);
                break;
            }
            case operator_type_t::subtract: {
                auto result_reg = instruction_block->current_target_register();

                auto lhs_reg = instruction_block->allocate_ireg();
                instruction_block->push_target_register(lhs_reg);
                _lhs->emit(r, assembler, context);
                instruction_block->pop_target_register();

                auto rhs_reg = instruction_block->allocate_ireg();
                instruction_block->push_target_register(rhs_reg);
                _rhs->emit(r, assembler, context);
                instruction_block->pop_target_register();

                instruction_block->sub_ireg_by_ireg_u64(
                    result_reg->reg.i,
                    lhs_reg,
                    rhs_reg);

                instruction_block->free_ireg(lhs_reg);
                instruction_block->free_ireg(rhs_reg);
                break;
            }
            case operator_type_t::multiply:
                break;
            case operator_type_t::divide:
                break;
            case operator_type_t::modulo:
                break;
            case operator_type_t::equals:
            case operator_type_t::less_than:
            case operator_type_t::not_equals:
            case operator_type_t::logical_or:
            case operator_type_t::logical_and:
            case operator_type_t::greater_than:
            case operator_type_t::less_than_or_equal:
            case operator_type_t::greater_than_or_equal: {
                emit_relational_operator(r, assembler, context, instruction_block);
                break;
            }
            case operator_type_t::exponent:
                break;
            case operator_type_t::binary_or:
                break;
            case operator_type_t::shift_left:
                break;
            case operator_type_t::binary_and:
                break;
            case operator_type_t::binary_xor:
                break;
            case operator_type_t::shift_right:
                break;
            case operator_type_t::rotate_right:
                break;
            case operator_type_t::rotate_left:
                break;
            case operator_type_t::assignment: {
                auto lhs_reg = instruction_block->allocate_ireg();
                instruction_block->push_target_register(lhs_reg);
                _lhs->emit(r, assembler, emit_context_t::for_write(context));
                instruction_block->pop_target_register();

                auto rhs_reg = instruction_block->allocate_ireg();
                instruction_block->push_target_register(rhs_reg);
                _rhs->emit(r, assembler, emit_context_t::for_read(context));
                instruction_block->pop_target_register();

                int64_t offset = 0;
                auto identifier = dynamic_cast<compiler::identifier*>(_lhs);
                if (identifier->stack_based()) {
                    lhs_reg = vm::i_registers_t::sp;
                    offset = -8;
                }

                instruction_block->store_from_ireg_u64(lhs_reg, rhs_reg, offset);
                instruction_block->pop_target_register();

                instruction_block->free_ireg(lhs_reg);
                instruction_block->free_ireg(rhs_reg);
                break;
            }
            default:
                break;
        }
        return true;
    }

    element* binary_operator::lhs() {
        return _lhs;
    }

    element* binary_operator::rhs() {
        return _rhs;
    }

    void binary_operator::emit_relational_operator(
            common::result& r,
            vm::assembler& assembler,
            const emit_context_t& context,
            vm::instruction_block* instruction_block) {
        auto lhs_reg = instruction_block->allocate_ireg();
        instruction_block->push_target_register(lhs_reg);
        _lhs->emit(r, assembler, context);
        instruction_block->pop_target_register();

        auto rhs_reg = instruction_block->allocate_ireg();
        instruction_block->push_target_register(rhs_reg);
        _rhs->emit(r, assembler, context);
        instruction_block->pop_target_register();

        instruction_block->cmp_u64(lhs_reg, rhs_reg);

        switch (operator_type()) {
            case operator_type_t::equals: {
                instruction_block->bne(context.data.if_data->false_branch_label);
                break;
            }
            case operator_type_t::less_than: {
                break;
            }
            case operator_type_t::not_equals: {
                break;
            }
            case operator_type_t::logical_or: {
                break;
            }
            case operator_type_t::logical_and: {
                break;
            }
            case operator_type_t::greater_than: {
                break;
            }
            case operator_type_t::less_than_or_equal: {
                break;
            }
            case operator_type_t::greater_than_or_equal: {
                break;
            }
            default: {
                break;
            }
        }

        instruction_block->free_ireg(lhs_reg);
        instruction_block->free_ireg(rhs_reg);
    }

    bool binary_operator::on_is_constant() const {
        return (_lhs != nullptr && _lhs->is_constant())
            && (_rhs != nullptr && _rhs->is_constant());
    }

    // XXX: this needs lots of future love
    compiler::type* binary_operator::on_infer_type(const compiler::program* program) {
        switch (operator_type()) {
            case operator_type_t::add:
            case operator_type_t::modulo:
            case operator_type_t::divide:
            case operator_type_t::subtract:
            case operator_type_t::multiply:
            case operator_type_t::exponent:
            case operator_type_t::binary_or:
            case operator_type_t::binary_and:
            case operator_type_t::binary_xor:
            case operator_type_t::shift_left:
            case operator_type_t::shift_right:
            case operator_type_t::rotate_left:
            case operator_type_t::rotate_right: {
                // XXX: this is SOOO not correct, but it gets us to the next step of
                //      code generation.
                return program->find_type_up("u64");
            }
            case operator_type_t::equals:
            case operator_type_t::less_than:
            case operator_type_t::not_equals:
            case operator_type_t::logical_or:
            case operator_type_t::logical_and:
            case operator_type_t::greater_than:
            case operator_type_t::less_than_or_equal:
            case operator_type_t::greater_than_or_equal: {
                return program->find_type_up("bool");
            }
            default:
                return nullptr;
        }
    }

};