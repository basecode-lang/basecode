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
#include "symbol_element.h"
#include "integer_literal.h"
#include "size_of_intrinsic.h"

namespace basecode::compiler {

    size_of_intrinsic::size_of_intrinsic(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::argument_list* args,
            compiler::procedure_type* proc_type,
            const compiler::type_reference_list_t& type_params) : intrinsic(
                                                                        module,
                                                                        parent_scope,
                                                                        args,
                                                                        proc_type,
                                                                        type_params) {
    }

    bool size_of_intrinsic::on_fold(
            compiler::session& session,
            fold_result_t& result) {
        auto args = arguments()->elements();
        if (args.empty() || args.size() > 1) {
            session.error(
                module(),
                "P091",
                "size_of expects a single argument.",
                location());
            return false;
        }

        auto& builder = session.builder();
        auto u32_type = session.scope_manager().find_type(qualified_symbol_t("u32"));

        infer_type_result_t infer_type_result {};
        if (args[0]->infer_type(session, infer_type_result)) {
            const auto& inferred = infer_type_result.types.back();
            result.element = builder.make_integer(
                parent_scope(),
                inferred.type->size_in_bytes(),
                builder.make_type_reference(
                    parent_scope(),
                    u32_type->symbol()->qualified_symbol(),
                    u32_type,
                    true));
            return true;
        }
        return false;
    }

    bool size_of_intrinsic::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.types.emplace_back(session.scope_manager().find_type(qualified_symbol_t("u32")));
        return true;
    }

    bool size_of_intrinsic::can_fold() const {
        return true;
    }

    bool size_of_intrinsic::on_is_constant() const {
        return true;
    }

    intrinsic_type_t size_of_intrinsic::type() const {
        return intrinsic_type_t::size_of;
    }

}