// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#include <vm/instruction_block.h>
#include "block.h"
#include "field.h"
#include "procedure_type.h"

namespace basecode::compiler {

    procedure_type::procedure_type(
        element* parent,
        compiler::block* scope,
        const std::string& name) : compiler::type(parent,
                                                  element_type_t::proc_type,
                                                  name),
                                   _scope(scope) {
    }

    bool procedure_type::on_emit(
            common::result& r,
            vm::assembler& assembler,
            const emit_context_t& context) {
        if (is_foreign())
            return true;

        auto instruction_block = assembler.make_procedure_block();
        auto proc_label = name();
        if (!context.procedure_identifier.empty())
            proc_label = context.procedure_identifier;
        instruction_block->make_label(proc_label);
        assembler.push_block(instruction_block);
        _scope->emit(r, assembler, context);
        assembler.pop_block();

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

    type_map_t& procedure_type::type_parameters() {
        return _type_parameters;
    }

    procedure_instance_list_t& procedure_type::instances() {
        return _instances;
    }

};