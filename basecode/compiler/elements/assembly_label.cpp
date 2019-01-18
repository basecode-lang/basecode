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
#include "assembly_label.h"
#include "identifier.h"
#include "pointer_type.h"
#include "symbol_element.h"
#include "type_reference.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    assembly_label::assembly_label(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::identifier_reference* ref) : element(module, parent_scope, element_type_t::assembly_label),
                                                   _ref(ref) {
    }

    bool assembly_label::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        auto& builder = session.builder();
        auto& scope_manager = session.scope_manager();

        compiler::type* type = nullptr;
        auto base_type = _ref != nullptr ?
                         _ref->identifier()->type_ref()->type() :
                         scope_manager.find_type(qualified_symbol_t("u0"));

        type = scope_manager.find_pointer_type(base_type);
        if (type == nullptr) {
            type = builder.make_pointer_type(
                parent_scope(),
                qualified_symbol_t(),
                base_type);
        }

        result.inferred_type = type;
        return true;
    }

    bool assembly_label::on_is_constant() const {
        return true;
    }

    compiler::identifier_reference* assembly_label::reference() {
        return _ref;
    }

};