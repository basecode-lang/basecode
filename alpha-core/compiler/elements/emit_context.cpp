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

#include "emit_context.h"

namespace basecode::compiler {

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

    void emit_context_t::pop_access() {
        if (access_stack.empty())
            return;
        access_stack.pop();
    }

    void emit_context_t::push_block(bool recurse) {
        data_stack.push(boost::any(block_data_t {
            .recurse = recurse
        }));
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

    emit_access_type_t emit_context_t::current_access() const {
        if (access_stack.empty())
            return emit_access_type_t::read;
        return access_stack.top();
    }

    void emit_context_t::push_access(emit_access_type_t type) {
        access_stack.push(type);
    }

    void emit_context_t::push_scratch_register(vm::i_registers_t reg) {
        scratch_registers.push(reg);
    }

};