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
#include <compiler/scope_manager.h>
#include <compiler/element_builder.h>
#include "type.h"
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

                const auto& lhs_inferred = lhs_type_result.types.back();
                const auto& rhs_inferred = rhs_type_result.types.back();

                auto lhs_size_in_bytes = lhs_inferred.type->size_in_bytes();
                auto rhs_size_in_bytes = rhs_inferred.type->size_in_bytes();

                if (rhs_size_in_bytes > lhs_size_in_bytes) {
                    result = rhs_type_result;
                } else {
                    result = lhs_type_result;
                }

                return true;
            }
            case operator_type_t::subscript: {
                infer_type_result_t type_result {};
                if (!_lhs->infer_type(session, type_result))
                    return false;

                const auto& inferred = type_result.types.back();
                auto ptr_type = dynamic_cast<compiler::pointer_type*>(inferred.type);
                if (ptr_type == nullptr)
                    return false;

                result.types.emplace_back(
                    ptr_type->base_type_ref()->type(),
                    ptr_type->base_type_ref());
                return true;
            }
            case operator_type_t::assignment: {
                infer_type_result_t rhs_type_result{};
                if (!_rhs->infer_type(session, rhs_type_result))
                    return false;

                infer_type_result_t lhs_type_result{};
                if (!_lhs->infer_type(session, lhs_type_result))
                    return false;

                size_t index = 0;
                for (const auto& rhs_type : rhs_type_result.types) {
                    if (rhs_type.type->is_unknown_type()) {
                        result.types.push_back(lhs_type_result.types[index]);
                    } else {
                        result.types.push_back(rhs_type);
                    }
                    ++index;
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
                result.types.emplace_back(
                    session
                        .scope_manager()
                        .find_type(qualified_symbol_t("bool"sv)));
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

    bool binary_operator::on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) {
        if (_lhs == e) {
            _lhs = fold_result.element;
        } else if (_rhs == e) {
            _rhs = fold_result.element;
        } else {
            // XXX: error
            return false;
        }
        return true;
    }

    compiler::element* binary_operator::lhs() {
        return _lhs;
    }

    compiler::element* binary_operator::rhs() {
        return _rhs;
    }

    compiler::element* binary_operator::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        return session.builder().make_binary_operator(
            new_scope,
            operator_type(),
            _lhs->clone<compiler::element>(session, new_scope),
            _rhs->clone<compiler::element>(session, new_scope));
    }

    bool binary_operator::on_is_constant() const {
        return (_lhs != nullptr && _lhs->is_constant())
            && (_rhs != nullptr && _rhs->is_constant());
    }

    std::string binary_operator::label_name() const {
        return fmt::format("bin_op_{}", id());
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

    bool binary_operator::on_as_integer(integer_result_t& result) const {
        integer_result_t lhs, rhs;
        if (!_rhs->as_integer(rhs)) return false;

        if (operator_type() == operator_type_t::member_access) {
            if (_lhs->element_type() == element_type_t::identifier_reference) {
                auto ref = dynamic_cast<compiler::identifier_reference*>(_lhs);
                auto composite_type = dynamic_cast<compiler::composite_type*>(ref->identifier()->type_ref()->type());
                if (composite_type->is_enum()) {
                    result = rhs;
                    return true;
                }
            }
            return false;
        }

        if (!_lhs->as_integer(lhs)) return false;

        switch (operator_type()) {
            case operator_type_t::add: {
                result.value = lhs.value + rhs.value;
                break;
            }
            case operator_type_t::divide: {
                result.value = lhs.value / rhs.value;
                break;
            }
            case operator_type_t::modulo: {
                result.value = lhs.value % rhs.value;
                break;
            }
            case operator_type_t::subtract: {
                result.value = lhs.value - rhs.value;
                break;
            }
            case operator_type_t::exponent: {
                result.value = common::power(lhs.value, rhs.value);
                break;
            }
            case operator_type_t::multiply: {
                result.value = lhs.value * rhs.value;
                break;
            }
            case operator_type_t::binary_or: {
                result.value = lhs.value | rhs.value;
                break;
            }
            case operator_type_t::binary_and: {
                result.value = lhs.value & rhs.value;
                break;
            }
            case operator_type_t::binary_xor: {
                result.value = lhs.value ^ rhs.value;
                break;
            }
            case operator_type_t::shift_left: {
                result.value = lhs.value << rhs.value;
                break;
            }
            case operator_type_t::shift_right: {
                result.value = lhs.value >> rhs.value;
                break;
            }
            case operator_type_t::rotate_left: {
                result.value = common::rotl(
                    lhs.value,
                    static_cast<uint8_t>(rhs.value));
                break;
            }
            case operator_type_t::rotate_right: {
                result.value = common::rotr(
                    lhs.value,
                    static_cast<uint8_t>(rhs.value));
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

}