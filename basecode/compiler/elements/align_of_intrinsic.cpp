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
#include "argument_list.h"
#include "integer_literal.h"
#include "align_of_intrinsic.h"

namespace basecode::compiler {

    align_of_intrinsic::align_of_intrinsic(
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

    bool align_of_intrinsic::on_fold(
            compiler::session& session,
            fold_result_t& result) {
        auto args = arguments()->elements();
        if (args.empty() || args.size() > 1) {
            session.error(
                module(),
                "P091",
                "align_of expects a single argument.",
                location());
            return false;
        }

        infer_type_result_t type_result {};
        if (args[0]->infer_type(session, type_result)) {
            result.element = session.builder().make_integer(
                parent_scope(),
                type_result.types.back().type->alignment());
            return true;
        }
        return false;
    }

    bool align_of_intrinsic::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.types.emplace_back(session.scope_manager().find_type(qualified_symbol_t("u32")));
        return true;
    }

    bool align_of_intrinsic::can_fold() const {
        return true;
    }

    bool align_of_intrinsic::on_is_constant() const {
        return true;
    }

    intrinsic_type_t align_of_intrinsic::type() const {
        return intrinsic_type_t::align_of;
    }

}