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
#include "label.h"
#include "block.h"
#include "switch_element.h"

namespace basecode::compiler {

    switch_element::switch_element(
        compiler::module* module,
        compiler::block* parent_scope,
        compiler::block* scope,
        compiler::element* expression) : element(module, parent_scope, element_type_t::switch_e),
                                         _scope(scope),
                                         _expr(expression) {
    }

    bool switch_element::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        auto begin_label_name = fmt::format("{}_begin", label_name());
        auto exit_label_name = fmt::format("{}_exit", label_name());
        auto end_label_name = fmt::format("{}_end", label_name());

        auto exit_label_ref = assembler.make_label_ref(exit_label_name);

        emit_context_t switch_context {};
        vm::control_flow_t flow_control {
            .exit_label = exit_label_ref,
        };
        flow_control.values.insert(std::make_pair(switch_expression, _expr));
        switch_context.flow_control = &flow_control;

        block->label(assembler.make_label(begin_label_name));
        _scope->emit(session, switch_context, result);

        block->label(assembler.make_label(exit_label_name));
        block->nop();
        block->label(assembler.make_label(end_label_name));

        return true;
    }

    compiler::block* switch_element::scope() {
        return _scope;
    }

    compiler::element* switch_element::expression() {
        return _expr;
    }

    void switch_element::on_owned_elements(element_list_t& list) {
        if (_scope != nullptr)
            list.emplace_back(_scope);

        if (_expr != nullptr)
            list.emplace_back(_expr);
    }

};