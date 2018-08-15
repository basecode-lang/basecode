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
#include "block.h"
#include "field.h"
#include "element.h"
#include "program.h"
#include "procedure_type.h"
#include "symbol_element.h"

namespace basecode::compiler {

    procedure_type::procedure_type(
            compiler::module* module,
            block* parent_scope,
            compiler::block* scope,
            compiler::symbol_element* symbol) : compiler::type(
                                                      module,
                                                      parent_scope,
                                                      element_type_t::proc_type,
                                                      symbol),
                                                _scope(scope) {
    }

    bool procedure_type::on_emit(compiler::session& session) {
        auto procedure_label = symbol()->name();
        auto parent_init = parent_element_as<compiler::initializer>();
        if (parent_init != nullptr) {
            auto parent_var = parent_init->parent_element_as<compiler::identifier>();
            if (parent_var != nullptr) {
                procedure_label = parent_var->symbol()->name();
            }
        }

        if (is_foreign()) {
            return true;
        }

        auto instruction_block = session.assembler().make_procedure_block();
        instruction_block->align(vm::instruction_t::alignment);
        instruction_block->current_entry()->blank_lines(1);
        instruction_block->memo();

        auto proc_label = instruction_block->make_label(procedure_label);
        instruction_block->current_entry()->label(proc_label);

        auto stack_frame = instruction_block->stack_frame();
        int32_t offset = -8;
        for (auto param : _parameters.as_list()) {
            stack_frame->add(
                vm::stack_frame_entry_type_t::parameter,
                param->identifier()->symbol()->name(),
                offset);
            offset -= 8;
        }

        offset = 8;
        const auto& returns_list = _returns.as_list();
        if (!returns_list.empty()) {
            stack_frame->add(
                vm::stack_frame_entry_type_t::return_slot,
                "return_value",
                offset);
            offset += 8;
        }

        offset = 16;
        size_t local_count = 0;
        session.program().visit_blocks(
            session.result(),
            [&](compiler::block* scope) {
                if (scope->element_type() == element_type_t::proc_type_block)
                    return true;
                for (auto var : scope->identifiers().as_list()) {
                    if (var->type()->element_type() == element_type_t::proc_type)
                        continue;
                    stack_frame->add(
                        vm::stack_frame_entry_type_t::local,
                        var->symbol()->name(),
                        offset);
                    var->usage(identifier_usage_t::stack);
                    offset += 8;
                    local_count++;
                }
                return true;
            },
            _scope);

        instruction_block->move_reg_to_reg(
            vm::register_t::fp(),
            vm::register_t::sp());
        auto size = 8 * local_count;
        if (!returns_list.empty())
            size += 8;
        if (size > 0) {
            instruction_block->sub_reg_by_immediate(
                vm::register_t::sp(),
                vm::register_t::sp(),
                size);
        }

        session.assembler().push_block(instruction_block);
        _scope->emit(session);
        session.assembler().pop_block();

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

    uint64_t procedure_type::foreign_address() const {
        return _foreign_address;
    }

    void procedure_type::foreign_address(uint64_t value) {
        _foreign_address = value;
    }

    procedure_instance_list_t& procedure_type::instances() {
        return _instances;
    }

    bool procedure_type::on_type_check(compiler::type* other) {
        if (other == nullptr)
            return false;

        return symbol()->name() == other->symbol()->name();
    }

    type_access_model_t procedure_type::on_access_model() const {
        return type_access_model_t::pointer;
    }

    void procedure_type::on_owned_elements(element_list_t& list) {
        if (_scope != nullptr)
            list.emplace_back(_scope);

        for (auto element : _returns.as_list())
            list.emplace_back(element);

        for (auto element : _parameters.as_list())
            list.emplace_back(element);
    }

    bool procedure_type::on_initialize(compiler::session& session) {
        return true;
    }

};