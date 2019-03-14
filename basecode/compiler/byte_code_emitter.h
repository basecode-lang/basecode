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

#pragma once

#include <vm/basic_block_map.h>
#include "variable_map.h"
#include "compiler_types.h"

namespace basecode::compiler {

    class byte_code_emitter {
    public:
        explicit byte_code_emitter(compiler::session& session);

        bool emit();

    // control flow stack
    private:
        void pop_flow_control();

        flow_control_t* current_flow_control();

        void push_flow_control(const flow_control_t& control_flow);

    private:
        bool emit_element(
            vm::basic_block** basic_block,
            compiler::element* e,
            emit_result_t& result);

        bool emit_type_info(
            vm::basic_block* block,
            compiler::type* type);

        bool emit_type_table();

        bool emit_section_variable(
            vm::basic_block* block,
            vm::section_t section,
            compiler::element* e);

        bool emit_section_tables();

        bool emit_procedure_types();

        void intern_string_literals();

        bool emit_interned_string_table();

        vm::basic_block* emit_bootstrap_block();

        std::string interned_string_data_label(common::id_t id);

        bool emit_end_block(const vm::basic_block_list_t& predecessors);

        vm::basic_block* emit_start_block(const vm::basic_block_list_t& predecessors);

        vm::basic_block* emit_implicit_blocks(const vm::basic_block_list_t& predecessors);

        // initializers & finalizers
    private:
        bool emit_finalizer(
            vm::basic_block* block,
            compiler::identifier* var);

        bool emit_initializer(
            vm::basic_block* block,
            compiler::identifier* var);

        bool emit_primitive_initializer(
            vm::basic_block* block,
            const vm::instruction_operand_t& base_local,
            compiler::identifier* var,
            int64_t offset);

        vm::basic_block* emit_finalizers(const vm::basic_block_list_t& predecessors);

        vm::basic_block* emit_initializers(const vm::basic_block_list_t& predecessors);

    // helper functions
    private:
        static compiler::element* find_call_site(compiler::procedure_call* proc_call);

        bool emit_block(
            vm::basic_block** basic_block,
            compiler::block* block);

        bool emit_arguments(
            vm::basic_block** basic_block,
            compiler::argument_list* arg_list,
            const compiler::element_list_t& elements);

        bool end_stack_frame(
            vm::basic_block** basic_block,
            compiler::block* block);

        bool begin_stack_frame(
            vm::basic_block** basic_block,
            compiler::block* block);

        bool emit_procedure_epilogue(
            vm::basic_block** basic_block,
            compiler::procedure_type* proc_type);

        bool emit_procedure_instance(
            vm::basic_block** basic_block,
            compiler::procedure_instance* proc_instance);

        bool emit_procedure_prologue(
            vm::basic_block** basic_block,
            compiler::procedure_type* proc_type);

        bool emit_arithmetic_operator(
            vm::basic_block** basic_block,
            compiler::binary_operator* binary_op,
            emit_result_t& result);

        bool emit_relational_operator(
            vm::basic_block** basic_block,
            compiler::binary_operator* binary_op,
            emit_result_t& result);

    private:
        variable_map _variables;
        compiler::session& _session;
        vm::basic_block_map _blocks {};
        vm::basic_block_stack_t _block_stack {};
        flow_control_stack_t _control_flow_stack {};
    };
};