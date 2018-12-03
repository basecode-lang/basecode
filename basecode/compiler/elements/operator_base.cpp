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
#include <compiler/scope_manager.h>
#include <compiler/element_builder.h>
#include "type.h"
#include "operator_base.h"
#include "float_literal.h"
#include "integer_literal.h"
#include "boolean_literal.h"

namespace basecode::compiler {

    operator_base::operator_base(
            compiler::module* module,
            block* parent_scope,
            element_type_t element_type,
            operator_type_t operator_type) : element(module, parent_scope, element_type),
                                             _operator_type(operator_type) {
    }

    bool operator_base::constant_fold_strategy(
            compiler::session& session,
            fold_result_t& result) {
        if (!is_constant())
            return false;

        auto builder = session.builder();
        auto scope_manager = session.scope_manager();

        infer_type_result_t type_result {};
        if (!infer_type(session, type_result))
            return false;

        if (type_result.inferred_type->number_class() == type_number_class_t::integer) {
            uint64_t value;
            if (as_integer(value)) {
                result.element = builder.make_integer(
                    scope_manager.current_scope(),
                    value);
                return true;
            }
        } else if (type_result.inferred_type->number_class() == type_number_class_t::floating_point) {
            double float_value;
            if (as_float(float_value)) {
                result.element = builder.make_float(
                    scope_manager.current_scope(),
                    float_value);
                return true;
            }
        }

        bool bool_value;
        if (as_bool(bool_value)) {
            result.element = bool_value ?
                             builder.true_literal() :
                             builder.false_literal();
            return true;
        }

        return false;
    }

    operator_type_t operator_base::operator_type() const {
        return _operator_type;
    }

};