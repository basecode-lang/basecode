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
#include "block.h"
#include "while_element.h"
#include "binary_operator.h"

namespace basecode::compiler {

    while_element::while_element(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::binary_operator* predicate,
            compiler::block* body) : element(module, parent_scope, element_type_t::while_e),
                                     _body(body),
                                     _predicate(predicate) {
    }

    compiler::block* while_element::body() {
        return _body;
    }

    compiler::binary_operator* while_element::predicate() {
        return _predicate;
    }

    bool while_element::on_emit(compiler::session& session) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        auto begin_label_name = fmt::format("{}_begin", label_name());
        auto body_label_name = fmt::format("{}_body", label_name());
        auto exit_label_name = fmt::format("{}_exit", label_name());
        auto end_label_name = fmt::format("{}_end", label_name());

        auto begin_label_ref = assembler.make_label_ref(begin_label_name);
        auto exit_label_ref = assembler.make_label_ref(exit_label_name);

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
        assembler.push_target_register(target_reg);
        _predicate->emit(session);
        assembler.pop_target_register();

        block->bz(target_reg, exit_label_ref);
        block->label(assembler.make_label(body_label_name));
        _body->emit(session);
        block->jump_direct(begin_label_ref);

        block->label(assembler.make_label(exit_label_name));
        block->nop();
        block->label(assembler.make_label(end_label_name));

        return true;
    }

    void while_element::on_owned_elements(element_list_t& list) {
        if (_body != nullptr)
            list.emplace_back(_body);

        if (_predicate != nullptr)
            list.emplace_back(_predicate);
    }

};