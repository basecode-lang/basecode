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

#include <compiler/elements/type.h>
#include <compiler/elements/identifier.h>
#include <compiler/elements/symbol_element.h>
#include <compiler/elements/string_literal.h>
#include <compiler/elements/identifier_reference.h>
#include "emit_context.h"

namespace basecode::compiler {

    bool variable_t::init(
            vm::assembler* assembler,
            vm::instruction_block* block) {
        if (address_loaded)
            return true;

        if (usage == identifier_usage_t::heap) {
            if (address_offset != 0) {
                block->move_label_to_ireg_with_offset(
                    address_reg,
                    name,
                    address_offset);
            } else {
                block->move_label_to_ireg(address_reg, name);
            }
            block
                ->current_entry()
                ->comment(fmt::format("identifier '{}' address (global)", name));
        }

        address_loaded = true;

        return true;
    }

    bool variable_t::read(
            vm::assembler* assembler,
            vm::instruction_block* block) {
        init(assembler, block);

        std::string type_name = "global";
        if (requires_read) {
            if (usage == identifier_usage_t::stack) {
                type_name = stack_frame_entry_type_name(frame_entry->type);
                block->load_to_ireg(
                    vm::op_sizes::qword,
                    value_reg.i,
                    vm::i_registers_t::fp,
                    frame_entry->offset);
            } else {
                block->load_to_ireg(
                    vm::op_size_for_byte_size(type->size_in_bytes()),
                    value_reg.i,
                    address_reg);
                block
                    ->current_entry()
                    ->comment(fmt::format("load identifier '{}' value ({})", name, type_name));
            }
            requires_read = false;
        }

        auto target_reg = assembler->current_target_register();
        if (target_reg != nullptr && target_reg->reg.i != value_reg.i) {
            block->move_ireg_to_ireg(target_reg->reg.i, value_reg.i);
            block
                ->current_entry()
                ->comment("assign target register to value register");
        }

        return true;
    }

    bool variable_t::write(
            vm::assembler* assembler,
            vm::instruction_block* block) {
        auto target_reg = assembler->current_target_register();
        if (target_reg == nullptr)
            return false;

        block->store_from_ireg(
            vm::op_size_for_byte_size(type->size_in_bytes()),
            address_reg,
            target_reg->reg.i,
            frame_entry != nullptr ? frame_entry->offset : 0);

        written = true;
        requires_read = true;

        return true;
    }

    ///////////////////////////////////////////////////////////////////////////

    emit_context_t::emit_context_t(
        vm::terp* terp,
        vm::assembler* assembler,
        compiler::program* program) : terp(terp),
                                      assembler(assembler),
                                      program(program) {
    }

    void emit_context_t::pop() {
        if (data_stack.empty())
            return;
        data_stack.pop();
    }

    void emit_context_t::push_if(
            const std::string& true_label_name,
            const std::string& false_label_name) {
        data_stack.push(boost::any(if_data_t {
            .true_branch_label = true_label_name,
            .false_branch_label = false_label_name,
        }));
    }

    variable_t* emit_context_t::allocate_variable(
            common::result& r,
            const std::string& name,
            compiler::type* type,
            identifier_usage_t usage,
            vm::stack_frame_entry_t* frame_entry) {
        auto var = variable(name);
        if (var != nullptr)
            return var;

        variable_t new_var {
            .name = name,
            .type = type,
            .usage = usage,
            .written = false,
            .requires_read = false,
            .address_loaded = false,
            .frame_entry = frame_entry,
        };

        if (usage == identifier_usage_t::heap) {
            if (!assembler->allocate_reg(new_var.address_reg)) {
            }
        }

        if (new_var.type->number_class() == type_number_class_t::integer) {
            if (!assembler->allocate_reg(new_var.value_reg.i)) {
            }
            new_var.requires_read = true;
        } else if (new_var.type->number_class() == type_number_class_t::floating_point) {
            if (!assembler->allocate_reg(new_var.value_reg.f)) {
            }
            new_var.requires_read = true;
        } else {
            // XXX: what happened here?  add error
        }
        auto it = variables.insert(std::make_pair(name, new_var));
        return it.second ? &it.first->second : nullptr;
    }

    void emit_context_t::clear_scratch_registers() {
        while (!scratch_registers.empty())
            scratch_registers.pop();
    }

    bool emit_context_t::has_scratch_register() const {
        return !scratch_registers.empty();
    }

    vm::i_registers_t emit_context_t::pop_scratch_register() {
        if (scratch_registers.empty())
            return vm::i_registers_t::i0;

        auto reg = scratch_registers.top();
        scratch_registers.pop();
        return reg;
    }

    void emit_context_t::free_variable(const std::string& name) {
        auto var = variable(name);
        if (var != nullptr) {
            if (var->usage == identifier_usage_t::heap)
                assembler->free_reg(var->address_reg);
            if (var->type->number_class() == type_number_class_t::integer) {
                assembler->free_reg(var->value_reg.i);
            } else {
                assembler->free_reg(var->value_reg.f);
            }
            variables.erase(name);
        }
    }

    variable_t* emit_context_t::variable(const std::string& name) {
        const auto it = variables.find(name);
        if (it == variables.end())
            return nullptr;
        return &it->second;
    }

    void emit_context_t::push_scratch_register(vm::i_registers_t reg) {
        scratch_registers.push(reg);
    }

    variable_t* emit_context_t::variable_for_element(compiler::element* element) {
        if (element == nullptr)
            return nullptr;
        switch (element->element_type()) {
            case element_type_t::identifier: {
                auto identifier = dynamic_cast<compiler::identifier*>(element);
                return variable(identifier->symbol()->name());
            }
            case element_type_t::string_literal: {
                auto string_literal = dynamic_cast<compiler::string_literal*>(element);
                return variable(string_literal->label_name());
            }
            case element_type_t::identifier_reference: {
                auto identifier = dynamic_cast<compiler::identifier_reference*>(element)->identifier();
                return variable(identifier->symbol()->name());
            }
            default:
                return nullptr;
        }
    }

};