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

#include <common/bytes.h>
#include <compiler/session.h>
#include <vm/instruction_block.h>
#include "block.h"
#include "field.h"
#include "element.h"
#include "program.h"
#include "identifier.h"
#include "declaration.h"
#include "initializer.h"
#include "procedure_type.h"
#include "symbol_element.h"
#include "type_reference.h"
#include "integer_literal.h"
#include "binary_operator.h"

namespace basecode::compiler {

    procedure_type::procedure_type(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::block* scope,
            compiler::symbol_element* symbol) : compiler::type(
                                                      module,
                                                      parent_scope,
                                                      element_type_t::proc_type,
                                                      symbol),
                                                _scope(scope) {
    }

    bool procedure_type::emit_epilogue(
            session& session,
            emit_context_t& context,
            emit_result_t& result) {
        if (is_foreign())
            return true;

        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        if (!_has_return) {
            block->rts();
        }

        return true;
    }

    bool procedure_type::emit_prologue(
            session& session,
            emit_context_t& context,
            emit_result_t& result) {
        if (is_foreign())
            return true;

        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        auto procedure_label = symbol()->name();
        auto parent_init = parent_element_as<compiler::initializer>();
        if (parent_init != nullptr) {
            auto parent_var = parent_init->parent_element_as<compiler::identifier>();
            if (parent_var != nullptr) {
                procedure_label = parent_var->label_name();
            }
        }

        block->blank_line();
        block->align(vm::instruction_t::alignment);
        block->label(assembler.make_label(procedure_label));

        auto& stack_offsets = _scope->stack_frame().offsets();
        stack_offsets.locals = 8;
        if (_return_type != nullptr) {
            auto entry = _scope->stack_frame().add(
                stack_frame_entry_type_t::return_slot,
                _return_type->identifier()->symbol()->name(),
                8);
            _return_type->identifier()->stack_frame_entry(entry);

            stack_offsets.return_slot = 16;
            stack_offsets.parameters = 24;
        } else {
            stack_offsets.parameters = 16;
        }

        auto fields = parameters().as_list();
        for (auto fld : fields) {
            auto var = fld->identifier();
            auto type = var->type_ref()->type();
            // XXX: if we change procedure_call to
            //      sub.qw sp, sp, {size}
            //
            //      and then store.x sp, {value}, offset
            //      we can use truer sizes within
            //      the 8-byte aligned stack block.
            //
            auto entry = _scope->stack_frame().add(
                stack_frame_entry_type_t::parameter,
                var->symbol()->name(),
                common::align(type->size_in_bytes(), 8));
            var->stack_frame_entry(entry);
        }

        return true;
    }

    bool procedure_type::has_return() const {
        return _has_return;
    }

    bool procedure_type::is_foreign() const {
        return _is_foreign;
    }

    compiler::block* procedure_type::scope() {
        return _scope;
    }

    field_map_t& procedure_type::parameters() {
        return _parameters;
    }

    bool procedure_type::is_proc_type() const {
        return true;
    }

    void procedure_type::is_foreign(bool value) {
        _is_foreign = value;
    }

    void procedure_type::has_return(bool value) {
        _has_return = value;
    }

    bool procedure_type::on_is_constant() const {
        return true;
    }

    type_map_t& procedure_type::type_parameters() {
        return _type_parameters;
    }

    compiler::field* procedure_type::return_type() {
        return _return_type;
    }

    uint64_t procedure_type::foreign_address() const {
        return _foreign_address;
    }

    void procedure_type::foreign_address(uint64_t value) {
        _foreign_address = value;
    }

    procedure_instance_list_t& procedure_type::instances() {
        return _instances;
    }

    void procedure_type::return_type(compiler::field* value) {
        _return_type = value;
    }

    compiler::procedure_instance* procedure_type::instance_for(
            compiler::session& session,
            compiler::procedure_call* call) {
        // XXX: this is not complete.  for testing only
        if (_instances.empty())
            return nullptr;
        return _instances.back();
    }

    bool procedure_type::on_type_check(compiler::type* other) {
        if (other == nullptr)
            return false;

        // XXX: very temporary hack...
        return other->element_type() == element_type_t::proc_type;
    }

    type_access_model_t procedure_type::on_access_model() const {
        return type_access_model_t::pointer;
    }

    void procedure_type::on_owned_elements(element_list_t& list) {
        if (_scope != nullptr)
            list.emplace_back(_scope);

        if (_return_type != nullptr)
            list.emplace_back(_return_type);

        for (auto element : _parameters.as_list())
            list.emplace_back(element);
    }

    bool procedure_type::on_initialize(compiler::session& session) {
        return true;
    }

};