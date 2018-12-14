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

    bool while_element::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        auto begin_label_name = fmt::format("{}_begin", label_name());
        auto body_label_name = fmt::format("{}_body", label_name());
        auto exit_label_name = fmt::format("{}_exit", label_name());
        auto end_label_name = fmt::format("{}_end", label_name());

        auto begin_label_ref = assembler.make_label_ref(begin_label_name);
        auto exit_label_ref = assembler.make_label_ref(exit_label_name);

        emit_context_t while_context {};
        vm::control_flow_t flow_control {
            .exit_label = exit_label_ref,
            .continue_label = begin_label_ref
        };
        while_context.flow_control = &flow_control;

        block->label(assembler.make_label(begin_label_name));

        emit_result_t predicate_result {};
        _predicate->emit(session, while_context, predicate_result);

        block->bz(
            predicate_result.operands.back(),
            vm::instruction_operand_t(exit_label_ref));

        block->label(assembler.make_label(body_label_name));
        _body->emit(session, while_context, result);
        block->jump_direct(begin_label_ref);

        block->label(assembler.make_label(exit_label_name));
        block->nop();
        block->label(assembler.make_label(end_label_name));

        return true;
    }

    compiler::block* while_element::body() {
        return _body;
    }

    bool while_element::on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) {
        _predicate = fold_result.element;
        return true;
    }

    compiler::element* while_element::predicate() {
        return _predicate;
    }

    void while_element::predicate(compiler::element* value) {
        // XXX: this new pointer is going to leak
        //      need to use a different collection that makes it easy
        //      to swap values.
        _predicate = value;
    }

    void while_element::on_owned_elements(element_list_t& list) {
        if (_body != nullptr)
            list.emplace_back(_body);

        if (_predicate != nullptr)
            list.emplace_back(_predicate);
    }

};