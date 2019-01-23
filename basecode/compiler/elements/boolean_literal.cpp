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
#include "boolean_literal.h"

namespace basecode::compiler {

    boolean_literal::boolean_literal(
            compiler::module* module,
            block* parent_scope,
            bool value) : element(module, parent_scope, element_type_t::boolean_literal),
                          _value(value) {
    }

    bool boolean_literal::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.inferred_type = session
            .scope_manager()
            .find_type(qualified_symbol_t("bool"));
        return true;
    }

    bool boolean_literal::value() const {
        return _value;
    }

    bool boolean_literal::on_is_constant() const {
        return true;
    }

    bool boolean_literal::on_as_bool(bool& value) const {
        value = _value;
        return true;
    }

    bool boolean_literal::on_equals(const compiler::element& other) const {
        auto other_bool = dynamic_cast<const compiler::boolean_literal*>(&other);
        return _value == other_bool->_value;
    }

    bool boolean_literal::on_not_equals(const compiler::element& other) const {
        auto other_bool = dynamic_cast<const compiler::boolean_literal*>(&other);
        return _value != other_bool->_value;
    }

};