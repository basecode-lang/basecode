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
#include "pointer_type.h"
#include "argument_list.h"
#include "symbol_element.h"
#include "type_reference.h"
#include "integer_literal.h"
#include "type_of_intrinsic.h"
#include "assembly_literal_label.h"

namespace basecode::compiler {

    type_of_intrinsic::type_of_intrinsic(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::argument_list* args,
            compiler::procedure_type* proc_type,
            const compiler::type_reference_list_t& type_params) : intrinsic(module,
                                                                            parent_scope,
                                                                            args,
                                                                            proc_type,
                                                                            type_params) {
    }

    bool type_of_intrinsic::on_fold(
            compiler::session& session,
            fold_result_t& result) {
        auto args = arguments()->elements();
        if (args.empty() || args.size() > 1) {
            session.error(
                module(),
                "P091",
                "type_of expects a single argument.",
                location());
            return false;
        }

        auto arg = args[0];
        infer_type_result_t arg_type_result {};
        if (!arg->infer_type(session, arg_type_result)) {
            session.error(
                module(),
                "P091",
                "type_of unable to infer type.",
                location());
            return false;
        }
        auto label_name = compiler::type::make_info_label_name(arg_type_result.inferred_type);

        infer_type_result_t type_result {};
        if (!infer_type(session, type_result)) {
            session.error(
                module(),
                "P091",
                "type_of unable to infer type.",
                location());
            return false;
        }

        result.element = session.builder().make_assembly_literal_label(
            parent_scope(),
            type_result.inferred_type,
            label_name,
            module());
        result.element->location(location());

        return true;
    }

    bool type_of_intrinsic::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        auto& builder = session.builder();
        auto& scope_manager = session.scope_manager();

        auto base_type = scope_manager.find_type(qualified_symbol_t("type"));
        auto type = scope_manager.find_pointer_type(base_type);
        if (type == nullptr) {
            type = builder.make_pointer_type(
                parent_scope(),
                qualified_symbol_t(),
                base_type);
        }
        result.inferred_type = type;

        return true;
    }

    bool type_of_intrinsic::can_fold() const {
        return true;
    }

    intrinsic_type_t type_of_intrinsic::type() const {
        return intrinsic_type_t::type_of;
    }

}