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
#include <compiler/scope_manager.h>
#include "type.h"
#include "argument_list.h"
#include "alloc_intrinsic.h"

namespace basecode::compiler {

    alloc_intrinsic::alloc_intrinsic(
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

    bool alloc_intrinsic::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

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

        variable_handle_t arg_var;
        if (!session.variable(arg, arg_var))
            return false;
        arg_var->read();

        vm::instruction_operand_t result_operand;
        if (!vm::instruction_operand_t::allocate(
                assembler,
                result_operand,
                vm::op_sizes::qword,
                vm::register_type_t::integer)) {
            return false;
        }

        result.operands.emplace_back(result_operand);

        block->alloc(
            vm::op_sizes::byte,
            result_operand,
            arg_var->emit_result().operands.back());

        return true;
    }

    bool alloc_intrinsic::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.inferred_type = session
            .scope_manager()
            .find_type(qualified_symbol_t("u64"));
        return true;
    }

    std::string alloc_intrinsic::name() const {
        return "alloc";
    }

};