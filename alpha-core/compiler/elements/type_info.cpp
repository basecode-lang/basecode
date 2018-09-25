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
#include <compiler/scope_manager.h>
#include "block.h"
#include "program.h"
#include "type_info.h"
#include "identifier.h"
#include "declaration.h"
#include "initializer.h"
#include "symbol_element.h"
#include "type_reference.h"
#include "assembly_label.h"

namespace basecode::compiler {

    type_info::type_info(
        compiler::module* module,
        compiler::block* parent_scope,
        compiler::block* scope) : compiler::composite_type(
                                    module,
                                    parent_scope,
                                    composite_types_t::struct_type,
                                    scope,
                                    nullptr,
                                    element_type_t::type_info) {
    }

    bool type_info::on_emit_initializer(
            compiler::session& session,
            compiler::identifier* var) {
        compiler::assembly_label* label = nullptr;
        auto init = var->initializer();
        if (init == nullptr)
            return true;

        label = dynamic_cast<compiler::assembly_label*>(init->expression());

        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        block->comment(
            fmt::format("initialize identifier: {}", var->symbol()->name()),
            4);

        auto work_var = session.variable_for_element(var);
        if (work_var == nullptr) {
            session.error(
                var,
                "P051",
                fmt::format("missing assembler variable for {}.", var->label_name()),
                var->location());
            return false;
        }

        work_var->make_live(session);
        defer({
            work_var->make_dormant(session);
        });

        var->emit(session);
        work_var->init(session);

        vm::register_t temp_reg;
        temp_reg.size = vm::op_sizes::qword;
        temp_reg.type = vm::register_type_t::integer;

        if (!assembler.allocate_reg(temp_reg)) {
            session.error(
                var,
                "P052",
                "assembler registers exhausted.",
                var->location());
            return false;
        }

        assembler.push_target_register(temp_reg);
        label->emit(session);
        assembler.pop_target_register();

        block->store_from_reg(work_var->address_reg.reg, temp_reg);

        return true;
    }

    type_access_model_t type_info::on_access_model() const {
        return type_access_model_t::pointer;
    }

    bool type_info::on_initialize(compiler::session& session) {
        auto& builder = session.builder();
        symbol(builder.make_symbol(parent_scope(), "type"));

        auto block_scope = scope();

        auto string_type = session.scope_manager().find_type({.name = "string"});
        auto string_type_ref = builder.make_type_reference(
            block_scope,
            string_type->symbol()->qualified_symbol(),
            string_type);

        auto name_identifier = builder.make_identifier(
            block_scope,
            builder.make_symbol(parent_scope(), "name"),
            nullptr);
        name_identifier->type_ref(string_type_ref);
        block_scope->identifiers().add(name_identifier);

        auto name_field = builder.make_field(
            this,
            block_scope,
            builder.make_declaration(block_scope, name_identifier, nullptr),
            0);

        fields().add(name_field);

        return composite_type::on_initialize(session);
    }

};