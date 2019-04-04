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
#include "nil_literal.h"
#include "pointer_type.h"

namespace basecode::compiler {

    nil_literal::nil_literal(
            compiler::module* module,
            block* parent_scope) : element(module, parent_scope, element_type_t::nil_literal) {
    }

    bool nil_literal::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        auto& builder = session.builder();
        auto& scope_manager = session.scope_manager();

        auto base_type = scope_manager.find_type(qualified_symbol_t("u0"));
        auto type = scope_manager.find_pointer_type(base_type);
        if (type == nullptr) {
            type = builder.make_pointer_type(
                parent_scope(),
                qualified_symbol_t(),
                base_type);
        }

        result.types.emplace_back(type);

        return true;
    }

    bool nil_literal::on_is_constant() const {
        return true;
    }

}