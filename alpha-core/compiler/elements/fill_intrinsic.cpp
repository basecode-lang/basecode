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
#include <vm/instruction_block.h>
#include "type.h"
#include "argument_list.h"
#include "type_reference.h"
#include "fill_intrinsic.h"

namespace basecode::compiler {

    fill_intrinsic::fill_intrinsic(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::argument_list* args) : intrinsic(module, parent_scope, args) {
    }

    std::string fill_intrinsic::name() const {
        return "fill";
    }

    bool fill_intrinsic::on_emit(compiler::session& session) {
        auto& assembler = session.assembler();
        auto instruction_block = assembler.current_block();

        auto args = arguments()->elements();
        if (args.empty() || args.size() < 3 || args.size() > 3) {
            session.error(
                this,
                "P091",
                "fill expects destination, value, and length arguments.",
                location());
            return false;
        }

        auto dest_arg = args[0];
        auto value_arg = args[1];
        auto length_arg = args[2];

//        auto dest_arg_reg = register_for(session, dest_arg);
//        if (dest_arg_reg.var != nullptr) {
//            dest_arg_reg.clean_up = true;
//        }
//
//        assembler.push_target_register(dest_arg_reg.reg);
//        dest_arg->emit(session);
//        assembler.pop_target_register();
//
//        auto value_arg_reg = register_for(session, value_arg);
//        if (value_arg_reg.var != nullptr) {
//            value_arg_reg.clean_up = true;
//        }
//
//        assembler.push_target_register(value_arg_reg.reg);
//        value_arg->emit(session);
//        assembler.pop_target_register();
//
//        auto length_arg_reg = register_for(session, length_arg);
//        if (length_arg_reg.var != nullptr) {
//            length_arg_reg.clean_up = true;
//        }
//
//        assembler.push_target_register(length_arg_reg.reg);
//        length_arg->emit(session);
//        assembler.pop_target_register();
//
//        instruction_block->fill(
//            vm::op_sizes::byte,
//            dest_arg_reg.reg,
//            value_arg_reg.reg,
//            length_arg_reg.reg);

        return true;
    }
    
};