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

#include <common/bytes.h>
#include <common/defer.h>
#include <compiler/session.h>
#include <vm/instruction_block.h>
#include "type.h"
#include "element.h"
#include "program.h"
#include "identifier.h"
#include "array_type.h"
#include "pointer_type.h"
#include "composite_type.h"
#include "symbol_element.h"
#include "type_reference.h"
#include "binary_operator.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    binary_operator::binary_operator(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::operator_type_t type,
            compiler::element* lhs,
            compiler::element* rhs) : operator_base(module, parent_scope, element_type_t::binary_operator, type),
                                      _lhs(lhs),
                                      _rhs(rhs) {
    }

    bool binary_operator::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        auto& assembler = session.assembler();

        auto block = assembler.current_block();
        block->label(assembler.make_label(fmt::format("{}_begin", label_name())));

        switch (operator_type()) {
            case operator_type_t::add:
            case operator_type_t::modulo:
            case operator_type_t::divide:
            case operator_type_t::subtract:
            case operator_type_t::multiply:
            case operator_type_t::exponent:
            case operator_type_t::binary_or:
            case operator_type_t::shift_left:
            case operator_type_t::binary_and:
            case operator_type_t::binary_xor:
            case operator_type_t::shift_right:
            case operator_type_t::rotate_left:
            case operator_type_t::rotate_right: {
                return emit_arithmetic_operator(
                    session,
                    context,
                    result);
            }
            case operator_type_t::equals:
            case operator_type_t::less_than:
            case operator_type_t::not_equals:
            case operator_type_t::logical_or:
            case operator_type_t::logical_and:
            case operator_type_t::greater_than:
            case operator_type_t::less_than_or_equal:
            case operator_type_t::greater_than_or_equal: {
                emit_relational_operator(
                    session,
                    context,
                    result);
                break;
            }
            case operator_type_t::subscript: {
                block->comment("XXX: implement subscript operator", 4);
                block->nop();
                break;
            }
            case operator_type_t::member_access: {
                variable_handle_t field_var {};
                if (!session.variable(this, field_var))
                    return false;

                auto type = field_var->type_result().inferred_type;
                auto target_number_class = type->number_class();
                auto target_size = type->size_in_bytes();
                auto target_type = target_number_class == type_number_class_t::integer ?
                                   vm::register_type_t::integer :
                                   vm::register_type_t::floating_point;

                vm::instruction_operand_t result_operand;
                if (!vm::instruction_operand_t::allocate(
                        assembler,
                        result_operand,
                        vm::op_size_for_byte_size(target_size),
                        target_type)) {
                    return false;
                }
                result.operands.emplace_back(result_operand);

                block->move(
                    result_operand,
                    field_var->emit_result().operands.back(),
                    vm::instruction_operand_t::empty());
                break;
            }
            case operator_type_t::assignment: {
                infer_type_result_t lhs_type {};
                if (!_lhs->infer_type(session, lhs_type))
                    return false;

                infer_type_result_t rhs_type {};
                if (!_rhs->infer_type(session, rhs_type))
                    return false;

                auto copy_required = false;
                auto lhs_is_composite = lhs_type.inferred_type->is_composite_type();
                auto rhs_is_composite = rhs_type.inferred_type->is_composite_type();

                if (!lhs_type.inferred_type->is_pointer_type()) {
                    if (lhs_is_composite && !rhs_is_composite) {
                        session.error(
                            _rhs,
                            "X000",
                            "cannot assign scalar to composite type.",
                            _rhs->location());
                        return false;
                    }

                    if (!lhs_is_composite && rhs_is_composite) {
                        session.error(
                            _rhs,
                            "X000",
                            "cannot assign composite type to scalar.",
                            _rhs->location());
                        return false;
                    }

                    copy_required = lhs_is_composite && rhs_is_composite;
                }

                variable_handle_t lhs_var;
                if (!session.variable(_lhs, lhs_var))
                    return false;

                variable_handle_t rhs_var;
                if (!session.variable(_rhs, rhs_var))
                    return false;

                if (copy_required) {
                    lhs_var->copy(
                        rhs_var.get(),
                        rhs_type.inferred_type->size_in_bytes());
                } else {
                    lhs_var->write(rhs_var.get());
                }
                break;
            }
            default:
                break;
        }

        return true;
    }

    bool binary_operator::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        switch (operator_type()) {
            case operator_type_t::add:
            case operator_type_t::modulo:
            case operator_type_t::divide:
            case operator_type_t::subtract:
            case operator_type_t::multiply:
            case operator_type_t::exponent:
            case operator_type_t::binary_or:
            case operator_type_t::subscript:
            case operator_type_t::binary_and:
            case operator_type_t::binary_xor:
            case operator_type_t::shift_left:
            case operator_type_t::shift_right:
            case operator_type_t::rotate_left:
            case operator_type_t::rotate_right: {
                infer_type_result_t lhs_type_result {};
                if (!_lhs->infer_type(session, lhs_type_result))
                    return false;

                infer_type_result_t rhs_type_result {};
                if (!_rhs->infer_type(session, rhs_type_result))
                    return false;

                auto lhs_size_in_bytes = lhs_type_result.inferred_type->size_in_bytes();
                auto rhs_size_in_bytes = rhs_type_result.inferred_type->size_in_bytes();

                if (rhs_size_in_bytes > lhs_size_in_bytes) {
                    result = rhs_type_result;
                } else {
                    result = lhs_type_result;
                }

                return true;
            }
            case operator_type_t::member_access: {
                return _rhs->infer_type(session, result);
            }
            case operator_type_t::equals:
            case operator_type_t::less_than:
            case operator_type_t::not_equals:
            case operator_type_t::logical_or:
            case operator_type_t::logical_and:
            case operator_type_t::greater_than:
            case operator_type_t::less_than_or_equal:
            case operator_type_t::greater_than_or_equal: {
                result.inferred_type = session
                    .scope_manager()
                    .find_type(qualified_symbol_t("bool"));
                return true;
            }
            default:
                return false;
        }
    }

    bool binary_operator::on_fold(
            compiler::session& session,
            fold_result_t& result) {
        return constant_fold_strategy(session, result);
    }

    compiler::element* binary_operator::lhs() {
        return _lhs;
    }

    compiler::element* binary_operator::rhs() {
        return _rhs;
    }

    bool binary_operator::on_is_constant() const {
        return (_lhs != nullptr && _lhs->is_constant())
            && (_rhs != nullptr && _rhs->is_constant());
    }

    bool binary_operator::on_as_bool(bool& value) const {
        value = false;

        switch (operator_type()) {
            case operator_type_t::equals: {
                value = *_lhs == *_rhs;
                break;
            }
            case operator_type_t::less_than: {
                value = *_lhs < *_rhs;
                break;
            }
            case operator_type_t::logical_or: {
                bool lhs, rhs;
                if (!_lhs->as_bool(lhs)) return false;
                if (!_rhs->as_bool(rhs)) return false;
                value = lhs || rhs;
                break;
            }
            case operator_type_t::logical_and: {
                bool lhs, rhs;
                if (!_lhs->as_bool(lhs)) return false;
                if (!_rhs->as_bool(rhs)) return false;
                value = lhs && rhs;
                break;
            }
            case operator_type_t::not_equals: {
                value = *_lhs != *_rhs;
                break;
            }
            case operator_type_t::greater_than: {
                value = *_lhs > *_rhs;
                break;
            }
            case operator_type_t::less_than_or_equal: {
                value = *_lhs <= *_rhs;
                break;
            }
            case operator_type_t::greater_than_or_equal: {
                value = *_lhs >= *_rhs;
                break;
            }
            default: return false;
        }

        return true;
    }

    void binary_operator::lhs(compiler::element* element) {
        _lhs = element;
    }

    void binary_operator::rhs(compiler::element* element) {
        _rhs = element;
    }

    bool binary_operator::on_as_float(double& value) const {
        double lhs_value, rhs_value;
        if (!_lhs->as_float(lhs_value)) return false;
        if (!_rhs->as_float(rhs_value)) return false;
        value = 0.0;

        switch (operator_type()) {
            case operator_type_t::add: {
                value = lhs_value + rhs_value;
                break;
            }
            case operator_type_t::divide: {
                value = lhs_value / rhs_value;
                break;
            }
            case operator_type_t::subtract: {
                value = lhs_value - rhs_value;
                break;
            }
            case operator_type_t::exponent: {
                value = std::pow(lhs_value, rhs_value);
                break;
            }
            case operator_type_t::multiply: {
                value = lhs_value * rhs_value;
                break;
            }
            default: return false;
        }

        return true;
    }

    bool binary_operator::on_as_integer(uint64_t& value) const {
        uint64_t lhs_value, rhs_value;
        if (!_lhs->as_integer(lhs_value)) return false;
        if (!_rhs->as_integer(rhs_value)) return false;
        value = 0;

        switch (operator_type()) {
            case operator_type_t::add: {
                value = lhs_value + rhs_value;
                break;
            }
            case operator_type_t::divide: {
                value = lhs_value / rhs_value;
                break;
            }
            case operator_type_t::modulo: {
                value = lhs_value % rhs_value;
                break;
            }
            case operator_type_t::subtract: {
                value = lhs_value - rhs_value;
                break;
            }
            case operator_type_t::exponent: {
                value = common::power(lhs_value, rhs_value);
                break;
            }
            case operator_type_t::multiply: {
                value = lhs_value * rhs_value;
                break;
            }
            case operator_type_t::binary_or: {
                value = lhs_value | rhs_value;
                break;
            }
            case operator_type_t::binary_and: {
                value = lhs_value & rhs_value;
                break;
            }
            case operator_type_t::binary_xor: {
                value = lhs_value ^ rhs_value;
                break;
            }
            case operator_type_t::shift_left: {
                value = lhs_value << rhs_value;
                break;
            }
            case operator_type_t::shift_right: {
                value = lhs_value >> rhs_value;
                break;
            }
            case operator_type_t::rotate_left: {
                value = common::rotl(
                    lhs_value,
                    static_cast<uint8_t>(rhs_value));
                break;
            }
            case operator_type_t::rotate_right: {
                value = common::rotr(
                    lhs_value,
                    static_cast<uint8_t>(rhs_value));
                break;
            }
            default: return false;
        }

        return true;
    }

    void binary_operator::on_owned_elements(element_list_t& list) {
        if (_lhs != nullptr)
            list.emplace_back(_lhs);
        if( _rhs != nullptr)
            list.emplace_back(_rhs);
    }

    void binary_operator::emit_relational_operator(
            compiler::session& session,
            emit_context_t& context,
            emit_result_t& result) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        auto end_label_name = fmt::format("{}_end", label_name());
        auto end_label_ref = assembler.make_label_ref(end_label_name);

        vm::instruction_operand_t result_operand;
        if (!vm::instruction_operand_t::allocate(
                assembler,
                result_operand,
                vm::op_sizes::byte,
                vm::register_type_t::integer)) {
            return;
        }
        result.operands.emplace_back(result_operand);

        variable_handle_t lhs_var;
        if (!session.variable(_lhs, lhs_var))
            return;
        lhs_var->read();

        variable_handle_t rhs_var;
        if (!session.variable(_rhs, rhs_var))
            return;

        auto is_signed = lhs_var->type_result().inferred_type->is_signed();

        if (is_logical_conjunction_operator(operator_type())) {
            switch (operator_type()) {
                case operator_type_t::logical_or: {
                    block->bnz(
                        lhs_var->emit_result().operands.back(),
                        vm::instruction_operand_t(end_label_ref));
                    break;
                }
                case operator_type_t::logical_and: {
                    block->bz(
                        lhs_var->emit_result().operands.back(),
                        vm::instruction_operand_t(end_label_ref));
                    break;
                }
                default: {
                    break;
                }
            }
            rhs_var->read();

            // XXX: this works but i'd like to eliminate the extra instruction
            block->move(
                result_operand,
                rhs_var->emit_result().operands.back());
        } else {
            rhs_var->read();

            block->cmp(
                lhs_var->emit_result().operands.back(),
                rhs_var->emit_result().operands.back());

            switch (operator_type()) {
                case operator_type_t::equals: {
                    block->setz(result_operand);
                    break;
                }
                case operator_type_t::less_than: {
                    if (is_signed)
                        block->setl(result_operand);
                    else
                        block->setb(result_operand);
                    break;
                }
                case operator_type_t::not_equals: {
                    block->setnz(result_operand);
                    break;
                }
                case operator_type_t::greater_than: {
                    if (is_signed)
                        block->setg(result_operand);
                    else
                        block->seta(result_operand);
                    break;
                }
                case operator_type_t::less_than_or_equal: {
                    if (is_signed)
                        block->setle(result_operand);
                    else
                        block->setbe(result_operand);
                    break;
                }
                case operator_type_t::greater_than_or_equal: {
                    if (is_signed)
                        block->setge(result_operand);
                    else
                        block->setae(result_operand);
                    break;
                }
                default: {
                    break;
                }
            }
        }

        block->label(assembler.make_label(end_label_name));
    }

    bool binary_operator::emit_arithmetic_operator(
            compiler::session& session,
            emit_context_t& context,
            emit_result_t& result) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        infer_type_result_t type_result {};
        if (!infer_type(session, type_result))
            return false;

        variable_handle_t lhs_var;
        if (!session.variable(_lhs, lhs_var))
            return false;
        lhs_var->read();

        variable_handle_t rhs_var;
        if (!session.variable(_rhs, rhs_var))
            return false;
        rhs_var->read();

        vm::instruction_operand_t result_operand;
        if (!vm::instruction_operand_t::allocate(
                assembler,
                result_operand,
                vm::op_size_for_byte_size(type_result.inferred_type->size_in_bytes()),
                lhs_var->value_reg().type)) {
            return false;
        }

        result.operands.emplace_back(result_operand);

        switch (operator_type()) {
            case operator_type_t::add: {
                block->add(
                    result_operand,
                    lhs_var->emit_result().operands.back(),
                    rhs_var->emit_result().operands.back());
                break;
            }
            case operator_type_t::divide: {
                block->div(
                    result_operand,
                    lhs_var->emit_result().operands.back(),
                    rhs_var->emit_result().operands.back());
                break;
            }
            case operator_type_t::modulo: {
                block->mod(
                    result_operand,
                    lhs_var->emit_result().operands.back(),
                    rhs_var->emit_result().operands.back());
                break;
            }
            case operator_type_t::multiply: {
                block->mul(
                    result_operand,
                    lhs_var->emit_result().operands.back(),
                    rhs_var->emit_result().operands.back());
                break;
            }
            case operator_type_t::exponent: {
                block->pow(
                    result_operand,
                    lhs_var->emit_result().operands.back(),
                    rhs_var->emit_result().operands.back());
                break;
            }
            case operator_type_t::subtract: {
                block->sub(
                    result_operand,
                    lhs_var->emit_result().operands.back(),
                    rhs_var->emit_result().operands.back());
                break;
            }
            case operator_type_t::binary_or: {
                block->or_op(
                    result_operand,
                    lhs_var->emit_result().operands.back(),
                    rhs_var->emit_result().operands.back());
                break;
            }
            case operator_type_t::shift_left: {
                block->shl(
                    result_operand,
                    lhs_var->emit_result().operands.back(),
                    rhs_var->emit_result().operands.back());
                break;
            }
            case operator_type_t::binary_and: {
                block->and_op(
                    result_operand,
                    lhs_var->emit_result().operands.back(),
                    rhs_var->emit_result().operands.back());
                break;
            }
            case operator_type_t::binary_xor: {
                block->xor_op(
                    result_operand,
                    lhs_var->emit_result().operands.back(),
                    rhs_var->emit_result().operands.back());
                break;
            }
            case operator_type_t::rotate_left: {
                block->rol(
                    result_operand,
                    lhs_var->emit_result().operands.back(),
                    rhs_var->emit_result().operands.back());
                break;
            }
            case operator_type_t::shift_right: {
                block->shr(
                    result_operand,
                    lhs_var->emit_result().operands.back(),
                    rhs_var->emit_result().operands.back());
                break;
            }
            case operator_type_t::rotate_right: {
                block->ror(
                    result_operand,
                    lhs_var->emit_result().operands.back(),
                    rhs_var->emit_result().operands.back());
                break;
            }
            default:
                break;
        }

        return true;
    }

};