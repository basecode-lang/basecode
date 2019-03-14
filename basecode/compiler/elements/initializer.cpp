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
#include "initializer.h"
#include "float_literal.h"
#include "procedure_type.h"
#include "binary_operator.h"
#include "integer_literal.h"

namespace basecode::compiler {

    initializer::initializer(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* expr) : element(module, parent_scope, element_type_t::initializer),
                                       _expr(expr) {
    }

    bool initializer::on_fold(
            compiler::session& session,
            fold_result_t& result) {
        if (_expr == nullptr)
            return false;

        auto& builder = session.builder();

        switch (_expr->element_type()) {
            case element_type_t::nil_literal:
            case element_type_t::boolean_literal: {
                result.element = _expr;
                break;
            }
            case element_type_t::float_literal: {
                result.element = builder.make_float(
                    _expr->parent_scope(),
                    dynamic_cast<compiler::float_literal*>(_expr)->value());
                break;
            }
            case element_type_t::integer_literal: {
                result.element = builder.make_integer(
                    _expr->parent_scope(),
                    dynamic_cast<compiler::integer_literal*>(_expr)->value());
                break;
            }
            default: {
                return false;
            }
        }

        return true;
    }

    bool initializer::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        if (_expr != nullptr) {
            return _expr->infer_type(session, result);
        }
        return false;
    }

    bool initializer::is_nil() const {
        return _expr != nullptr
               && _expr->element_type() == element_type_t::nil_literal;
    }

    bool initializer::on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) {
        _expr = fold_result.element;
        return true;
    }

    bool initializer::on_is_constant() const {
        if (_expr == nullptr)
            return false;
        return _expr->is_constant();
    }

    compiler::element* initializer::expression() {
        return _expr;
    }

    bool initializer::on_as_bool(bool& value) const {
        if (_expr == nullptr)
            return false;
        return _expr->as_bool(value);
    }

    bool initializer::on_as_float(double& value) const {
        if (_expr == nullptr)
            return false;
        return _expr->as_float(value);
    }

    bool initializer::on_as_integer(uint64_t& value) const {
        if (_expr == nullptr)
            return false;
        return _expr->as_integer(value);
    }

    void initializer::expression(compiler::element* value) {
        _expr = value;
    }

    compiler::procedure_type* initializer::procedure_type() {
        if (_expr == nullptr || _expr->element_type() != element_type_t::proc_type)
            return nullptr;
        return dynamic_cast<compiler::procedure_type*>(_expr);
    }

    bool initializer::on_as_string(std::string& value) const {
        if (_expr == nullptr)
            return false;
        return _expr->as_string(value);
    }

    void initializer::on_owned_elements(element_list_t& list) {
        if (_expr != nullptr)
            list.emplace_back(_expr);
    }

    bool initializer::on_as_rune(common::rune_t& value) const {
        if (_expr == nullptr)
            return false;
        return _expr->as_rune(value);
    }

}