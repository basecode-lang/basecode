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
#include "element.h"

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
            emit_context_t& context) {
        if (is_foreign())
            return true;

        auto instruction_block = assembler.make_procedure_block();
        auto procedure_label = name();
        auto proc_type_data = context.top<procedure_type_data_t>();
        if (proc_type_data != nullptr)
            procedure_label = proc_type_data->identifier_name;

        instruction_block->make_label(procedure_label);
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