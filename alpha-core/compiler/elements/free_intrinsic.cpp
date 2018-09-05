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
#include "free_intrinsic.h"

namespace basecode::compiler {

    free_intrinsic::free_intrinsic(
            compiler::module* module,
            block* parent_scope,
            argument_list* args) : intrinsic(module, parent_scope, args) {
    }

    std::string free_intrinsic::name() const {
        return "free";
    }

    bool free_intrinsic::on_emit(compiler::session& session) {
        auto& assembler = session.assembler();
        auto instruction_block = assembler.current_block();

        auto args = arguments()->elements();
        if (args.empty() || args.size() > 1) {
            session.error(
                this,
                "P091",
                "free expects an u64 or pointer argument.",
                location());
            return false;
        }

        auto arg = args[0];
        infer_type_result_t infer_type_result {};
        if (!arg->infer_type(session, infer_type_result)) {
            // XXX: error
            return false;
        }
        if (infer_type_result.inferred_type->number_class() != type_number_class_t::integer) {
            session.error(
                this,
                "P091",
                "free expects an u64 or pointer argument.",
                location());
            return false;
        }

        auto arg_reg = register_for(session, arg);
        if (arg_reg.var != nullptr) {
            arg_reg.clean_up = true;
        }

        assembler.push_target_register(arg_reg.reg);
        arg->emit(session);
        assembler.pop_target_register();

        instruction_block->free(arg_reg.reg);

        return true;
    }

};