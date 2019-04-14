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
#include <compiler/scope_manager.h>
#include "numeric_type.h"
#include "type_reference.h"
#include "integer_literal.h"

namespace basecode::compiler {

    integer_literal::integer_literal(
            compiler::module* module,
            block* parent_scope,
            uint64_t value,
            compiler::type_reference* type_ref,
            bool is_signed) : element(module, parent_scope, element_type_t::integer_literal),
                              _value(value),
                              _is_signed(is_signed),
                              _type_ref(type_ref) {
    }

    bool integer_literal::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        if (_type_ref != nullptr) {
            result.types.emplace_back(_type_ref->type(), _type_ref);
        } else {
            result.types.emplace_back(
                session
                    .scope_manager()
                    .find_type(qualified_symbol_t(numeric_type::narrow_to_value(_value))));
        }
        return true;
    }

    void integer_literal::value(uint64_t v) {
        _value = v;
    }

    bool integer_literal::is_signed() const {
        return _is_signed;
    }

    uint64_t integer_literal::value() const {
        return _value;
    }

    bool integer_literal::on_is_constant() const {
        return true;
    }

    bool integer_literal::is_sign_bit_high() const {
        return common::is_sign_bit_set(_value);
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

}