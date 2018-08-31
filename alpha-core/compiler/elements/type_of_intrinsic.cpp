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

        auto arg_type = args[0]->infer_type(session);
        result.element = session.builder().make_integer(
            parent_scope(),
            0xdeadbeef);
        return true;
    }

    std::string type_of_intrinsic::name() const {
        return "type_of";
    }

    bool type_of_intrinsic::on_is_constant() const {
        return true;
    }

    compiler::type* type_of_intrinsic::on_infer_type(const compiler::session& session) {
        auto& scope_manager = session.scope_manager();
        auto type_info_type = scope_manager.find_type(qualified_symbol_t {
            .name = "type"
        });
        auto ptr_type = scope_manager.find_pointer_type(
            type_info_type,
            parent_scope());
        if (ptr_type == nullptr) {
            ptr_type = const_cast<compiler::session&>(session)
                .builder()
                .make_pointer_type(parent_scope(), type_info_type);
        }
        return ptr_type;
    }

};