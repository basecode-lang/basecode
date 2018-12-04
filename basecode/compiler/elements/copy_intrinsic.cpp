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
            compiler::argument_list* args,
            compiler::procedure_type* proc_type,
            const compiler::type_reference_list_t& type_params) : intrinsic(module,
                                                                            parent_scope,
                                                                            args,
                                                                            proc_type,
                                                                            type_params) {
    }

    std::string copy_intrinsic::name() const {
        return "copy";
    }

    bool copy_intrinsic::on_emit(compiler::session& session) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        auto args = arguments()->elements();
        if (args.empty() || args.size() < 3 || args.size() > 3) {
            session.error(
                this,
                "P091",
                "copy expects destination, source, and size arguments.",
                location());
            return false;
        }

        variable_handle_t dest_var;
        if (!session.variable(args[0], dest_var))
            return false;
        dest_var->read();

        variable_handle_t src_var;
        if (!session.variable(args[1], src_var))
            return false;
        src_var->read();

        variable_handle_t size_var;
        if (!session.variable(args[2], size_var))
            return false;
        size_var->read();

        block->copy(
            vm::op_sizes::byte,
            dest_var->value_reg(),
            src_var->value_reg(),
            size_var->value_reg());

        return true;
    }

};