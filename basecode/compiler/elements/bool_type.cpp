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
#include "program.h"
#include "bool_type.h"
#include "identifier.h"
#include "initializer.h"
#include "boolean_literal.h"
#include "binary_operator.h"

namespace basecode::compiler {

    bool_type::bool_type(
        compiler::module* module,
        compiler::block* parent_scope) : compiler::type(
                                             module,
                                             parent_scope,
                                             element_type_t::bool_type,
                                             nullptr) {
    }

    bool bool_type::on_emit_initializer(
            compiler::session& session,
            compiler::variable* var) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        auto var_ident = dynamic_cast<compiler::identifier*>(var->element());
        auto init = var_ident->initializer();

        block->comment("initializer", vm::comment_location_t::after_instruction);
        if (init != nullptr) {
            variable_handle_t init_var{};
            if (!session.variable(init, init_var))
                return false;
            var->write(init_var.get());
        } else {
            var->write(var->value_reg().size, 0);
        }

        return true;
    }

    bool bool_type::on_type_check(compiler::type* other) {
        return other != nullptr
            && other->element_type() == element_type_t::bool_type;
    }

    type_access_model_t bool_type::on_access_model() const {
        return type_access_model_t::value;
    }

    type_number_class_t bool_type::on_number_class() const {
        return type_number_class_t::integer;
    }

    bool bool_type::on_initialize(compiler::session& session) {
        symbol(session.builder().make_symbol(parent_scope(), "bool"));
        size_in_bytes(1);
        return true;
    }

};