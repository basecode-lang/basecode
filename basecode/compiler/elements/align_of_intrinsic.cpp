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
#include "argument_list.h"
#include "integer_literal.h"
#include "align_of_intrinsic.h"

namespace basecode::compiler {

    align_of_intrinsic::align_of_intrinsic(
            compiler::module* module,
            block* parent_scope,
            argument_list* args) : intrinsic(module, parent_scope, args) {
    }

    bool align_of_intrinsic::on_fold(
            compiler::session& session,
            fold_result_t& result) {
        auto args = arguments()->elements();
        if (args.empty() || args.size() > 1) {
            session.error(
                this,
                "P091",
                "align_of expects a single argument.",
                location());
            return false;
        }

        infer_type_result_t infer_type_result {};
        if (args[0]->infer_type(session, infer_type_result)) {
            result.element = session.builder().make_integer(
                parent_scope(),
                infer_type_result.inferred_type->alignment());
            return true;
        }
        return false;
    }

    bool align_of_intrinsic::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.inferred_type = session.scope_manager().find_type(qualified_symbol_t {
            .name = "u32"
        });
        return true;
    }

    bool align_of_intrinsic::can_fold() const {
        return true;
    }

    std::string align_of_intrinsic::name() const {
        return "align_of";
    }

    bool align_of_intrinsic::on_is_constant() const {
        return true;
    }

};