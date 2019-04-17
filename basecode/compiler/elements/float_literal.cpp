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
#include "numeric_type.h"
#include "float_literal.h"
#include "type_reference.h"

namespace basecode::compiler {

    float_literal::float_literal(
            compiler::module* module,
            block* parent_scope,
            double value,
            compiler::type_reference* type_ref) : element(module, parent_scope, element_type_t::float_literal),
                                                  _value(value),
                                                  _type_ref(type_ref) {
    }

    bool float_literal::on_infer_type(
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

    double float_literal::value() const {
        return _value;
    }

    bool float_literal::is_signed() const {
        return _value < 0;
    }

    compiler::element* float_literal::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        return session
            .builder()
            .make_float(new_scope, _value, _type_ref);
    }

    bool float_literal::on_is_constant() const {
        return true;
    }

    bool float_literal::on_as_float(double& value) const {
        value = _value;
        return true;
    }

}