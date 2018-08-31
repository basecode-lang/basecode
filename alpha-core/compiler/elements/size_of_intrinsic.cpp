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
#include "size_of_intrinsic.h"

namespace basecode::compiler {

    size_of_intrinsic::size_of_intrinsic(
            compiler::module* module,
            block* parent_scope,
            argument_list* args) : intrinsic(module, parent_scope, args) {
    }

    bool size_of_intrinsic::on_fold(
            compiler::session& session,
            fold_result_t& result) {
        auto args = arguments()->elements();
        if (args.empty() || args.size() > 1) {
            session.error(
                this,
                "P091",
                "size_of expects a single argument.",
                location());
            return false;
        }

        auto arg_type = args[0]->infer_type(session);
        result.element = session.builder().make_integer(
            parent_scope(),
            arg_type->size_in_bytes());
        return true;
    }

    std::string size_of_intrinsic::name() const {
        return "size_of";
    }

    bool size_of_intrinsic::on_is_constant() const {
        return true;
    }

    compiler::type* size_of_intrinsic::on_infer_type(const compiler::session& session) {
        return session.scope_manager().find_type(qualified_symbol_t {
            .name = "u32"
        });
    }

};