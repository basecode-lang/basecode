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

    bool variable_register_t::reserve(vm::assembler* assembler) {
        if (integer)
            allocated = assembler->allocate_reg(value.i);
        else
            allocated = assembler->allocate_reg(value.f);
        return allocated;
    }

    void variable_register_t::release(vm::assembler* assembler) {
        if (!allocated)
            return;
        if (integer)
            assembler->free_reg(value.i);
        else
            assembler->free_reg(value.f);
        allocated = false;
    }

    ///////////////////////////////////////////////////////////////////////////

    bool variable_t::init(
            vm::assembler* assembler,
            vm::instruction_block* block) {
        if (!live)
            return false;

        if (address_loaded)
            return true;

        if (usage == identifier_usage_t::heap) {
            if (!address_reg.reserve(assembler))
                return false;

            if (address_offset != 0) {
                block->move_label_to_ireg_with_offset(
                    address_reg.value.i,
                    name,
                    address_offset);
            } else {
                block->move_label_to_ireg(address_reg.value.i, name);
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
        if (!live)
            return false;

        if (!init(assembler, block))
            return false;

        std::string type_name = "global";
        if (requires_read) {
            if (!value_reg.reserve(assembler))
                return false;

            if (usage == identifier_usage_t::stack) {
                type_name = stack_frame_entry_type_name(frame_entry->type);
                block->load_to_ireg(
                    vm::op_sizes::qword,
                    value_reg.value.i,
                    vm::i_registers_t::fp,
                    frame_entry->offset);
            } else {
                block->load_to_ireg(
                    vm::op_size_for_byte_size(type->size_in_bytes()),
                    value_reg.value.i,
                    address_reg.value.i);
                block
                    ->current_entry()
                    ->comment(fmt::format("load identifier '{}' value ({})", name, type_name));
            }
            requires_read = false;
        }

//        auto target_reg = assembler->current_target_register();
//        if (target_reg != nullptr && target_reg->reg.i != value_reg.value.i) {
//            block->move_ireg_to_ireg(target_reg->reg.i, value_reg.value.i);
//            block
//                ->current_entry()
//                ->comment("assign target register to value register");
//        }

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
            address_reg.value.i,
            target_reg->reg.i,
            frame_entry != nullptr ? frame_entry->offset : 0);

        written = true;
        requires_read = true;

        return true;
    }

    void variable_t::make_live(vm::assembler* assembler) {
        if (live)
            return;
        live = true;
        address_loaded = false;
        requires_read = type->access_model() != type_access_model_t::pointer;
    }

    void variable_t::make_dormat(vm::assembler* assembler) {
        if (!live)
            return;
        live = false;
        requires_read = false;
        address_loaded = false;
        value_reg.release(assembler);
        address_reg.release(assembler);
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

        auto it = variables.insert(std::make_pair(
            name,
            variable_t {
                .name = name,
                .type = type,
                .usage = usage,
                .written = false,
                .requires_read = false,
                .address_loaded = false,
                .frame_entry = frame_entry,
            }));
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
            var->make_dormat(assembler);
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