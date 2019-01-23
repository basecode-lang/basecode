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

#include "compiler_types.h"

namespace basecode::compiler {

    struct temp_count_result_t {
        size_t ints = 0;
        size_t floats = 0;

        void update();

        void count(compiler::type* type);

        temp_count_result_t& operator+ (const temp_count_result_t& rhs) {
            ints += rhs.ints;
            floats += rhs.floats;
            return *this;
        }

    private:
        size_t _ints = 0;
        size_t _floats = 0;
    };

    struct temp_local_t {
        std::string name;
        int64_t offset;
        vm::op_sizes size;
        vm::local_type_t type;
    };

    using temp_local_list_t = std::vector<temp_local_t>;

    ///////////////////////////////////////////////////////////////////////////

    class byte_code_emitter {
    public:
        explicit byte_code_emitter(compiler::session& session);

        bool emit();

    // control flow stack
    private:
        void pop_flow_control();

        flow_control_t* current_flow_control();

        void push_flow_control(const flow_control_t& control_flow);

    // instruction block stack
    private:
        vm::instruction_block* pop_block();

        vm::instruction_block* current_block();

        void push_block(vm::instruction_block* block);

    private:
        bool emit_element(
            vm::instruction_block* block,
            compiler::element* e,
            emit_result_t& result);

        bool emit_type_info(
            vm::instruction_block* block,
            compiler::type* type);

        bool count_temps(
            compiler::element* e,
            temp_count_result_t& result);

        bool make_temp_locals(
            compiler::block* block,
            temp_local_list_t& locals);

        bool emit_end_block();

        bool emit_type_table();

        bool emit_start_block();

        bool emit_section_variable(
            vm::instruction_block* block,
            vm::section_t section,
            compiler::element* e);

        bool emit_bootstrap_block();

        bool emit_procedure_types();

        bool emit_implicit_blocks();

        void intern_string_literals();

        element_list_t* variable_section(
            identifier_by_section_t& groups,
            vm::section_t section);

        bool emit_interned_string_table();

        bool group_identifiers(identifier_by_section_t& vars);

        bool emit_section_tables(identifier_by_section_t& vars);

        std::string interned_string_data_label(common::id_t id);

    // initializers & finalizers
    private:
        bool emit_finalizer(
            vm::instruction_block* block,
            compiler::identifier* var);

        bool emit_initializer(
            vm::instruction_block* block,
            compiler::identifier* var);

        bool emit_primitive_initializer(
            vm::instruction_block* block,
            const vm::instruction_operand_t& base_local,
            compiler::identifier* var,
            int64_t offset);

        bool emit_finalizers(identifier_by_section_t& vars);

        bool emit_initializers(identifier_by_section_t& vars);

    // helper functions
    private:
        void read(
            vm::instruction_block* block,
            emit_result_t& result,
            uint8_t number);

        bool emit_block(
            vm::instruction_block* basic_block,
            compiler::block* block,
            identifier_list_t& locals,
            temp_local_list_t& temp_locals);

        bool emit_arguments(
            vm::instruction_block* block,
            compiler::argument_list* arg_list,
            const compiler::element_list_t& elements);

        bool end_stack_frame(
            vm::instruction_block* basic_block,
            compiler::block* block,
            const identifier_list_t& locals);

        bool begin_stack_frame(
            vm::instruction_block* basic_block,
            compiler::block* block,
            identifier_list_t& locals,
            temp_local_list_t& temp_locals);

        std::string temp_local_name(
            number_class_t type,
            uint8_t number);

        bool emit_procedure_epilogue(
            vm::instruction_block* block,
            compiler::procedure_type* proc_type);

        bool emit_procedure_prologue(
            vm::instruction_block* block,
            compiler::procedure_type* proc_type,
            identifier_list_t& parameters);

        bool emit_arithmetic_operator(
            vm::instruction_block* block,
            compiler::binary_operator* binary_op,
            emit_result_t& result);

        bool emit_relational_operator(
            vm::instruction_block* block,
            compiler::binary_operator* binary_op,
            emit_result_t& result);

        bool is_temp_local(const vm::instruction_operand_t& operand);

    private:
        compiler::session& _session;
        basic_block_stack_t _block_stack {};
        flow_control_stack_t _control_flow_stack {};
    };
};