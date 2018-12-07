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
#include <compiler/session.h>
#include <vm/instruction_block.h>
#include "program.h"
#include "numeric_type.h"
#include "integer_literal.h"

namespace basecode::compiler {

    integer_literal::integer_literal(
            compiler::module* module,
            block* parent_scope,
            uint64_t value) : element(module, parent_scope, element_type_t::integer_literal),
                              _value(value) {
    }

    bool integer_literal::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();
        auto target_reg = assembler.current_target_register();
        if (target_reg != nullptr) {
            block->clr(vm::op_sizes::qword, *target_reg);
            block->move_constant_to_reg(*target_reg, _value);
        }
        return true;
    }

    bool integer_literal::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.inferred_type = session.scope_manager().find_type({
            .name = numeric_type::narrow_to_value(_value)
        });
        return true;
    }

    bool integer_literal::is_signed() const {
        return common::is_sign_bit_set(_value);
    }

    uint64_t integer_literal::value() const {
        return _value;
    }

    bool integer_literal::on_is_constant() const {
        return true;
    }

    bool integer_literal::on_as_integer(uint64_t& value) const {
        value = _value;
        return true;
    }

    bool integer_literal::on_equals(const compiler::element& other) const {
        auto other_int = dynamic_cast<const compiler::integer_literal*>(&other);
        return _value == other_int->_value;
    }

    uint64_t integer_literal::on_add(const compiler::element& other) const {
        auto other_int = dynamic_cast<const compiler::integer_literal*>(&other);
        return _value + other_int->_value;
    }

    bool integer_literal::on_less_than(const compiler::element& other) const {
        auto other_int = dynamic_cast<const compiler::integer_literal*>(&other);
        return _value < other_int->_value;
    }

    bool integer_literal::on_not_equals(const compiler::element& other) const {
        auto other_int = dynamic_cast<const compiler::integer_literal*>(&other);
        return _value != other_int->_value;
    }

    uint64_t integer_literal::on_subtract(const compiler::element& other) const {
        auto other_int = dynamic_cast<const compiler::integer_literal*>(&other);
        return _value - other_int->_value;
    }

    uint64_t integer_literal::on_multiply(const compiler::element& other) const {
        auto other_int = dynamic_cast<const compiler::integer_literal*>(&other);
        return _value * other_int->_value;
    }

    bool integer_literal::on_greater_than(const compiler::element& other) const {
        auto other_int = dynamic_cast<const compiler::integer_literal*>(&other);
        return _value > other_int->_value;
    }

    bool integer_literal::on_less_than_or_equal(const compiler::element& other) const {
        auto other_int = dynamic_cast<const compiler::integer_literal*>(&other);
        return _value <= other_int->_value;
    }

    bool integer_literal::on_greater_than_or_equal(const compiler::element& other) const {
        auto other_int = dynamic_cast<const compiler::integer_literal*>(&other);
        return _value >= other_int->_value;
    }

};