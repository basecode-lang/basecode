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
#include "type.h"
#include "identifier.h"
#include "initializer.h"
#include "symbol_element.h"
#include "type_reference.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    identifier_reference::identifier_reference(
            compiler::module* module,
            block* parent_scope,
            const qualified_symbol_t& symbol,
            compiler::identifier* identifier) : element(module, parent_scope, element_type_t::identifier_reference),
                                                _symbol(symbol),
                                                _identifier(identifier) {
    }

    bool identifier_reference::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        if (_identifier == nullptr)
            return false;
        return _identifier->emit(session, context, result);
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

    bool identifier_reference::on_as_integer(uint64_t& value) const {
        if (_identifier == nullptr)
            return false;
        return _identifier->as_integer(value);
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
                uint64_t lhs_value, rhs_value;
                if (!as_integer(lhs_value)) return false;
                if (!other.as_integer(rhs_value)) return false;
                return lhs_value == rhs_value;
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
                uint64_t lhs_value, rhs_value;
                if (!as_integer(lhs_value)) return false;
                if (!other.as_integer(rhs_value)) return false;
                return lhs_value < rhs_value;
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
                uint64_t lhs_value, rhs_value;
                if (!as_integer(lhs_value)) return false;
                if (!other.as_integer(rhs_value)) return false;
                return lhs_value > rhs_value;
            }
            default: {
                break;
            }
        }

        return false;
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

};