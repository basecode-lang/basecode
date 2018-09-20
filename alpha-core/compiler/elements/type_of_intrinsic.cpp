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
#include "type.h"
#include "pointer_type.h"
#include "argument_list.h"
#include "symbol_element.h"
#include "type_reference.h"
#include "assembly_label.h"
#include "integer_literal.h"
#include "type_of_intrinsic.h"

namespace basecode::compiler {

    type_of_intrinsic::type_of_intrinsic(
            compiler::module* module,
            block* parent_scope,
            argument_list* args) : intrinsic(module, parent_scope, args) {
    }

    bool type_of_intrinsic::on_fold(
            compiler::session& session,
            fold_result_t& result) {
        auto args = arguments()->elements();
        if (args.empty() || args.size() > 1) {
            session.error(
                this,
                "P091",
                "type_of expects a single argument.",
                location());
            return false;
        }

        auto arg = args[0];
        infer_type_result_t infer_type_result {};
        if (!arg->infer_type(session, infer_type_result)) {
            session.error(
                this,
                "P091",
                "type_of unable to infer type.",
                location());
            return false;
        }

        auto label_name = fmt::format(
            "_ti_{}",
            infer_type_result.inferred_type->symbol()->name());

        result.element = session.builder().make_assembly_label(
            parent_scope(),
            label_name);

        return true;
    }

    bool type_of_intrinsic::on_infer_type(
            const compiler::session& session,
            infer_type_result_t& result) {
        auto& scope_manager = session.scope_manager();
        qualified_symbol_t type_name = {
            .name = "type"
        };
        auto type_info_type = scope_manager.find_type(type_name);
        auto ptr_type = scope_manager.find_pointer_type(
            type_info_type,
            parent_scope());
        if (ptr_type == nullptr) {
            ptr_type = const_cast<compiler::session&>(session)
                .builder()
                .make_pointer_type(parent_scope(), type_name, type_info_type);
        }
        result.inferred_type = ptr_type;
        return true;
    }

    std::string type_of_intrinsic::name() const {
        return "type_of";
    }

    bool type_of_intrinsic::on_is_constant() const {
        return true;
    }

};