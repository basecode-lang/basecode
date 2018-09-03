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
#include "alloc_intrinsic.h"

namespace basecode::compiler {

    alloc_intrinsic::alloc_intrinsic(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::argument_list* args) : intrinsic(module, parent_scope, args) {
    }

    bool alloc_intrinsic::on_infer_type(
            const compiler::session& session,
            infer_type_result_t& result) {
        result.inferred_type = session.scope_manager().find_type(qualified_symbol_t {
            .name = "u64"
        });
        return true;
    }

    std::string alloc_intrinsic::name() const {
        return "alloc";
    }

    bool alloc_intrinsic::on_emit(compiler::session& session) {
        auto& assembler = session.assembler();
        auto instruction_block = assembler.current_block();
        auto target_reg = assembler.current_target_register();

        auto args = arguments()->elements();
        if (args.empty() || args.size() > 1) {
            session.error(
                this,
                "P091",
                "alloc expects a single integer argument.",
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
                "alloc expects a single integer argument.",
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

        instruction_block->alloc(vm::op_sizes::byte, *target_reg, arg_reg.reg);

        return true;
    }

};