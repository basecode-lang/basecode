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
#include "type.h"
#include "block.h"
#include "for_element.h"
#include "declaration.h"
#include "type_reference.h"
#include "binary_operator.h"
#include "integer_literal.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    for_element::for_element(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::declaration* induction_decl,
            compiler::element* expression,
            compiler::block* body) : element(module, parent_scope, element_type_t::for_e),
                                     _body(body),
                                     _expression(expression),
                                     _induction_decl(induction_decl) {
    }

    compiler::block* for_element::body() {
        return _body;
    }

    compiler::element* for_element::expression() {
        return _expression;
    }

    compiler::declaration* for_element::induction_decl() {
        return _induction_decl;
    }

    bool for_element::on_emit(compiler::session& session) {
        auto& builder = session.builder();
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        auto begin_label_name = fmt::format("{}_begin", label_name());
        auto body_label_name = fmt::format("{}_body", label_name());
        auto exit_label_name = fmt::format("{}_exit", label_name());

        auto begin_label_ref = assembler.make_label_ref(begin_label_name);
        auto exit_label_ref = assembler.make_label_ref(exit_label_name);

        auto induction_var_size_in_bytes = _induction_decl
            ->identifier()
            ->type_ref()
            ->type()
            ->size_in_bytes();
        assembler.push_control_flow(vm::control_flow_t {
            .exit_label = exit_label_ref,
            .continue_label = begin_label_ref
        });

        vm::register_t target_reg {
            .size = vm::op_sizes::byte,
            .type = vm::register_type_t::integer
        };
        assembler.allocate_reg(target_reg);
        defer({
            assembler.free_reg(target_reg);
            assembler.pop_control_flow();
        });

        block->label(assembler.make_label(begin_label_name));
        block->push(vm::register_t::fp());
        block->sub_reg_by_immediate(
            vm::register_t::fp(),
            vm::register_t::fp(),
            induction_var_size_in_bytes);

        assembler.push_target_register(target_reg);
        auto equals_op = builder.make_binary_operator(
            parent_scope(),
            operator_type_t::less_than,
            _induction_decl->identifier(),
            builder.make_integer(parent_scope(), 10)); // this is high
        equals_op->make_non_owning();
        defer(session.elements().remove(equals_op->id()));
        equals_op->emit(session);
        assembler.pop_target_register();
        block->bz(target_reg, exit_label_ref);

        block->label(assembler.make_label(body_label_name));
        _body->emit(session);

        block->jump_direct(begin_label_ref);

        block->label(assembler.make_label(exit_label_name));
        block->pop(vm::register_t::fp());

        return true;
    }

    void for_element::on_owned_elements(element_list_t& list) {
        if (_body != nullptr)
            list.emplace_back(_body);

        if (_expression != nullptr)
            list.emplace_back(_expression);

        if (_induction_decl != nullptr)
            list.emplace_back(_induction_decl);
    }

};