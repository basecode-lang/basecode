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
#include "copy_intrinsic.h"
#include "type_reference.h"

namespace basecode::compiler {

    copy_intrinsic::copy_intrinsic(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::argument_list* args) : intrinsic(module, parent_scope, args) {
    }

    std::string copy_intrinsic::name() const {
        return "copy";
    }

    bool copy_intrinsic::on_emit(compiler::session& session) {
        auto& assembler = session.assembler();
        auto instruction_block = assembler.current_block();

        auto args = arguments()->elements();
        if (args.empty() || args.size() < 3 || args.size() > 3) {
            session.error(
                this,
                "P091",
                "copy expects destination, source, and size arguments.",
                location());
            return false;
        }

        auto dest_arg = args[0];
        auto src_arg = args[1];
        auto size_arg = args[2];

//        auto dest_arg_reg = register_for(session, dest_arg);
//        if (dest_arg_reg.var != nullptr) {
//            dest_arg_reg.clean_up = true;
//        }
//
//        assembler.push_target_register(dest_arg_reg.reg);
//        dest_arg->emit(session);
//        assembler.pop_target_register();
//
//        auto src_arg_reg = register_for(session, src_arg);
//        if (src_arg_reg.var != nullptr) {
//            src_arg_reg.clean_up = true;
//        }
//
//        assembler.push_target_register(src_arg_reg.reg);
//        src_arg->emit(session);
//        assembler.pop_target_register();
//
//        auto size_arg_reg = register_for(session, size_arg);
//        if (size_arg_reg.var != nullptr) {
//            size_arg_reg.clean_up = true;
//        }
//
//        assembler.push_target_register(size_arg_reg.reg);
//        size_arg->emit(session);
//        assembler.pop_target_register();
//
//        instruction_block->copy(
//            vm::op_sizes::byte,
//            dest_arg_reg.reg,
//            src_arg_reg.reg,
//            size_arg_reg.reg);

        return true;
    }

};