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

    bool alloc_intrinsic::on_emit(compiler::session& session) {
        auto& assembler = session.assembler();
        auto instruction_block = assembler.current_block();
        auto target_reg = assembler.current_target_register();

        auto args = arguments()->elements();
        // XXX: needs error handling
        auto arg = args[0];

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

    compiler::type* alloc_intrinsic::on_infer_type(const compiler::session& session) {
        return session.scope_manager().find_type(qualified_symbol_t {
            .name = "u64"
        });
    }

};