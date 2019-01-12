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

    using identifier_by_section_t = std::map<vm::section_t, element_list_t>;

    class byte_code_emitter {
    public:
        explicit byte_code_emitter(compiler::session& session);

        bool emit();

        std::string interned_string_data_label(common::id_t id);

    private:
        bool emit_element(
            compiler::element* e,
            emit_result_t& result);

        bool emit_type_info(
            vm::instruction_block* block,
            compiler::type* type);

        bool emit_end_block();

        bool emit_type_table();

        bool emit_start_block();

        bool emit_section_variable(
            vm::instruction_block* block,
            compiler::element* e);

        bool emit_bootstrap_block();

        bool emit_procedure_types();

        bool emit_implicit_blocks();

        void intern_string_literals();

        bool emit_identifier_initializer(
            vm::instruction_block* block,
            compiler::identifier* var);

        element_list_t* variable_section(
            identifier_by_section_t& groups,
            vm::section_t section);

        bool emit_interned_string_table();

        bool emit_finalizers(identifier_by_section_t& vars);

        bool emit_initializers(identifier_by_section_t& vars);

        bool group_identifiers(identifier_by_section_t& vars);

        bool emit_section_tables(identifier_by_section_t& vars);

    private:
        compiler::session& _session;
    };
};