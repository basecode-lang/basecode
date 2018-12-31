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
#include "rune_type.h"
#include "identifier.h"
#include "initializer.h"

namespace basecode::compiler {

    rune_type::rune_type(
            compiler::module* module,
            block* parent_scope) : compiler::type(
                                       module,
                                       parent_scope,
                                       element_type_t::rune_type,
                                       nullptr) {
    }

    bool rune_type::on_emit_initializer(
            compiler::session& session,
            compiler::variable* var) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        auto var_ident = dynamic_cast<compiler::identifier*>(var->element());
        auto init = var_ident->initializer();

        block->comment(
            "initializer: rune",
            vm::comment_location_t::after_instruction);
        if (init != nullptr) {
            variable_handle_t init_var{};
            if (!session.variable(init, init_var))
                return false;
            var->write(init_var.get());
        } else {
            var->write(var->value_reg().size, common::rune_invalid);
        }

        return true;
    }

    bool rune_type::on_type_check(compiler::type* other) {
        return other != nullptr
               && other->element_type() == element_type_t::rune_type;
    }

    type_access_model_t rune_type::on_access_model() const {
        return type_access_model_t::value;
    }

    type_number_class_t rune_type::on_number_class() const {
        return type_number_class_t::integer;
    }

    bool rune_type::on_initialize(compiler::session& session) {
        symbol(session.builder().make_symbol(parent_scope(), "rune"));
        alignment(4);
        size_in_bytes(4);
        return true;
    }

};