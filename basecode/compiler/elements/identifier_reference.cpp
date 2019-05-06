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
#include <compiler/element_builder.h>
#include "type.h"
#include "field.h"
#include "identifier.h"
#include "initializer.h"
#include "unary_operator.h"
#include "symbol_element.h"
#include "type_reference.h"
#include "composite_type.h"
#include "binary_operator.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    void identifier_reference::evaluate_member_access_expression(
            compiler::element* e,
            binary_operator_stack_t& bin_op_stack,
            identifier_reference_stack_t& ref_stack) {
        switch (e->element_type()) {
            case element_type_t::binary_operator: {
                auto bin_op = dynamic_cast<compiler::binary_operator*>(e);
                if (bin_op->operator_type() == operator_type_t::member_access)
                    bin_op_stack.push(bin_op);
                break;
            }
            case element_type_t::identifier_reference: {
                auto ref = dynamic_cast<compiler::identifier_reference*>(e);
                ref_stack.push(ref);
                break;
            }
            default: {
                break;
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    identifier_reference::identifier_reference(
            compiler::module* module,
            block* parent_scope,
            const qualified_symbol_t& symbol,
            compiler::identifier* identifier) : element(module, parent_scope, element_type_t::identifier_reference),
                                                _symbol(symbol),
                                                _identifier(identifier) {
    }

    bool identifier_reference::on_fold(
            compiler::session& session,
            fold_result_t& result) {
        if (_identifier == nullptr)
            return false;
        return _identifier->fold(session, result);
    }

    bool identifier_reference::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        if (_identifier != nullptr)
            return _identifier->infer_type(session, result);
        return false;
    }

    bool identifier_reference::resolved() const {
        return _identifier != nullptr;
    }

    compiler::element* identifier_reference::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        return session.builder().make_identifier_reference(
            new_scope,
            _symbol,
            _identifier,
            true);
    }

    bool identifier_reference::on_is_constant() const {
        if (_identifier == nullptr)
            return false;
        return _identifier->is_constant();
    }

    std::string identifier_reference::label_name() const {
        if (_identifier != nullptr)
            return _identifier->label_name();
        return element::label_name();
    }

    offset_result_t identifier_reference::field_offset() {
        offset_result_t result{};

        binary_operator_stack_t bin_op_stack{};
        identifier_reference_stack_t ref_stack{};

        auto bin_op = dynamic_cast<compiler::binary_operator*>(parent_element());
        if (bin_op != nullptr && bin_op->operator_type() == operator_type_t::member_access) {
            if (bin_op->lhs()->id() == id())
                return result;
            bin_op_stack.push(bin_op);
        }

        while (!bin_op_stack.empty()) {
            bin_op = bin_op_stack.top();
            bin_op_stack.pop();

            evaluate_member_access_expression(bin_op->rhs(), bin_op_stack, ref_stack);
            evaluate_member_access_expression(bin_op->lhs(), bin_op_stack, ref_stack);
        }

        if (!ref_stack.empty()) {
            result.base_ref = ref_stack.top();
            result.path += result.base_ref->symbol().name;
            ref_stack.pop();

            auto composite_type = dynamic_cast<compiler::composite_type*>(result.base_ref->identifier()->type_ref()->type());
            if (composite_type != nullptr) {
                composite_type->calculate_size();
            }

            while (!ref_stack.empty()) {
                auto ref = ref_stack.top();
                ref_stack.pop();

                result.path += fmt::format(".{}", ref->symbol().name);

                auto field = ref->identifier()->field();
                if (field != nullptr) {
                    result.fields.emplace_back(field);
                    result.from_end += field->end_offset();
                    result.from_start += field->start_offset();
                }
            }
        }

        return result;
    }

    bool identifier_reference::on_as_bool(bool& value) const {
        if (_identifier == nullptr)
            return false;
        return _identifier->as_bool(value);
    }

    compiler::identifier* identifier_reference::identifier() {
        return _identifier;
    }

    bool identifier_reference::on_as_float(double& value) const {
        if (_identifier == nullptr)
            return false;
        return _identifier->as_float(value);
    }

    const qualified_symbol_t& identifier_reference::symbol() const {
        return _symbol;
    }

    bool identifier_reference::on_equals(const element& other) const {
        if (!_identifier->is_constant())
            return false;

        switch (_identifier->initializer()->expression()->element_type()) {
            case element_type_t::float_literal: {
                double lhs_value, rhs_value;
                if (!as_float(lhs_value)) return false;
                if (!other.as_float(rhs_value)) return false;
                return lhs_value == rhs_value;
            }
            case element_type_t::integer_literal: {
                integer_result_t lhs, rhs;
                if (!as_integer(lhs)) return false;
                if (!other.as_integer(rhs)) return false;
                return lhs.value == rhs.value;
            }
            case element_type_t::boolean_literal: {
                bool lhs_value, rhs_value;
                if (!as_bool(lhs_value)) return false;
                if (!other.as_bool(rhs_value)) return false;
                return lhs_value == rhs_value;
            }
            default: {
                break;
            }
        }

        return false;
    }

    bool identifier_reference::on_as_string(std::string& value) const {
        if (_identifier == nullptr)
            return false;
        return _identifier->as_string(value);
    }

    void identifier_reference::identifier(compiler::identifier* value) {
        _identifier = value;
    }

    bool identifier_reference::on_as_rune(common::rune_t& value) const {
        if (_identifier == nullptr)
            return false;
        return _identifier->as_rune(value);
    }

    bool identifier_reference::on_less_than(const element& other) const {
        if (!_identifier->is_constant())
            return false;

        switch (_identifier->initializer()->expression()->element_type()) {
            case element_type_t::float_literal: {
                double lhs_value, rhs_value;
                if (!as_float(lhs_value)) return false;
                if (!other.as_float(rhs_value)) return false;
                return lhs_value < rhs_value;
            }
            case element_type_t::integer_literal: {
                integer_result_t lhs, rhs;
                if (!as_integer(lhs)) return false;
                if (!other.as_integer(rhs)) return false;
                return lhs.value < rhs.value;
            }
            default: {
                break;
            }
        }

        return false;
    }

    bool identifier_reference::on_not_equals(const element& other) const {
        return !on_equals(other);
    }

    bool identifier_reference::on_greater_than(const element& other) const {
        if (!_identifier->is_constant())
            return false;

        switch (_identifier->initializer()->expression()->element_type()) {
            case element_type_t::float_literal: {
                double lhs_value, rhs_value;
                if (!as_float(lhs_value)) return false;
                if (!other.as_float(rhs_value)) return false;
                return lhs_value > rhs_value;
            }
            case element_type_t::integer_literal: {
                integer_result_t lhs, rhs;
                if (!as_integer(lhs)) return false;
                if (!other.as_integer(rhs)) return false;
                return lhs.value > rhs.value;
            }
            default: {
                break;
            }
        }

        return false;
    }

    bool identifier_reference::on_as_integer(integer_result_t& result) const {
        if (_identifier == nullptr)
            return false;
        return _identifier->as_integer(result);
    }

    bool identifier_reference::on_less_than_or_equal(const element& other) const {
        return on_less_than(other) || on_equals(other);
    }

    bool identifier_reference::on_greater_than_or_equal(const element& other) const {
        return on_greater_than(other) || on_equals(other);
    }

    bool identifier_reference::on_as_identifier(compiler::identifier*& value) const {
        value = _identifier;
        return true;
    }

}