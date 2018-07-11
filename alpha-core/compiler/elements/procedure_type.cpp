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

#include <vm/instruction_block.h>
#include "block.h"
#include "field.h"
#include "element.h"
#include "program.h"
#include "procedure_type.h"

namespace basecode::compiler {

    procedure_type::procedure_type(
        block* parent_scope,
        compiler::block* scope,
        const std::string& name) : compiler::type(parent_scope,
                                                  element_type_t::proc_type,
                                                  name),
                                   _scope(scope) {
    }

    bool procedure_type::on_emit(
            common::result& r,
            emit_context_t& context) {
        if (is_foreign())
            return true;

        auto instruction_block = context.assembler->make_procedure_block();
        auto procedure_label = name();
        auto proc_type_data = context.top<procedure_type_data_t>();
        if (proc_type_data != nullptr)
            procedure_label = proc_type_data->identifier_name;
        instruction_block->make_label(procedure_label);

        auto stack_frame = instruction_block->stack_frame();
        int32_t offset = -8;
        for (auto param : _parameters.as_list()) {
            stack_frame->add(
                vm::stack_frame_entry_type_t::parameter,
                param->identifier()->name(),
                offset);
            offset -= 8;
        }

        offset = 8;
        for (auto return_param : _returns.as_list()) {
            stack_frame->add(
                vm::stack_frame_entry_type_t::return_slot,
                "return_value",
                offset);
            offset += 8;
        }

        offset = 16;
        size_t local_count = 0;
        context.program->visit_blocks(
            r,
            [&](compiler::block* scope) {
                if (scope->element_type() == element_type_t::proc_type_block)
                    return true;
                for (auto var : scope->identifiers().as_list()) {
                    stack_frame->add(
                        vm::stack_frame_entry_type_t::local,
                        var->name(),
                        offset);
                    offset += 8;
                    local_count++;
                }
                return true;
            },
            _scope);

        instruction_block->move_ireg_to_ireg(
            vm::i_registers_t::fp,
            vm::i_registers_t::sp);
        if (local_count > 0) {
            instruction_block->sub_ireg_by_immediate(
                vm::i_registers_t::sp,
                vm::i_registers_t::sp,
                8 * local_count);
        }

        context.assembler->push_block(instruction_block);
        _scope->emit(r, context);
        context.assembler->pop_block();


        return true;
    }

    bool procedure_type::on_initialize(
            common::result& r,
            compiler::program* program) {
        return true;
    }

    bool procedure_type::is_foreign() const {
        return _is_foreign;
    }

    field_map_t& procedure_type::returns() {
        return _returns;
    }

    compiler::block* procedure_type::scope() {
        return _scope;
    }

    field_map_t& procedure_type::parameters() {
        return _parameters;
    }

    void procedure_type::is_foreign(bool value) {
        _is_foreign = value;
    }

    bool procedure_type::on_is_constant() const {
        return true;
    }

    type_map_t& procedure_type::type_parameters() {
        return _type_parameters;
    }

    procedure_instance_list_t& procedure_type::instances() {
        return _instances;
    }

};