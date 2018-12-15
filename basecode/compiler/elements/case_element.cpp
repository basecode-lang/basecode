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
#include "statement.h"
#include "case_element.h"
#include "binary_operator.h"

namespace basecode::compiler {

    case_element::case_element(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::block* scope,
            compiler::element* expr) : element(module, parent_scope, element_type_t::case_e),
                                       _scope(scope),
                                       _expr(expr) {
    }

    bool case_element::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        auto& builder = session.builder();
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        auto true_label_name = fmt::format("{}_true", label_name());
        auto false_label_name = fmt::format("{}_false", label_name());

        if (context.flow_control == nullptr) {
            // XXX: error
            return false;
        }

        context.flow_control->fallthrough = false;

        auto is_default_case = _expr == nullptr;

        vm::label_ref_t* fallthrough_label = nullptr;
        if (!is_default_case) {
            auto next = boost::any_cast<compiler::element*>(context.flow_control->values[next_element]);
            if (next != nullptr
            && next->element_type() == element_type_t::statement) {
                auto stmt = dynamic_cast<compiler::statement*>(next);
                if (stmt != nullptr
                && stmt->expression()->element_type() == element_type_t::case_e) {
                    auto next_case = dynamic_cast<compiler::case_element*>(stmt->expression());
                    auto next_true_label_name = fmt::format("{}_true", next_case->label_name());
                    fallthrough_label = assembler.make_label_ref(next_true_label_name);
                }
            }
        }

        if (!is_default_case) {
            auto switch_expr = boost::any_cast<compiler::element*>(context.flow_control->values[switch_expression]);
            auto equals_op = builder.make_binary_operator(
                parent_scope(),
                operator_type_t::equals,
                switch_expr,
                _expr);
            equals_op->make_non_owning();
            defer(session.elements().remove(equals_op->id()));

            emit_result_t equals_result(assembler);
            equals_op->emit(session, context, equals_result);
            block->bz(
                equals_result.operands.back(),
                vm::instruction_operand_t(assembler.make_label_ref(false_label_name)));
        }

        block->label(assembler.make_label(true_label_name));
        _scope->emit(session, context, result);

        if (!is_default_case) {
            if (context.flow_control->fallthrough) {
                block->jump_direct(fallthrough_label);
            } else {
                block->jump_direct(context.flow_control->exit_label);
            }
        }

        block->label(assembler.make_label(false_label_name));

        return true;
    }

    compiler::block* case_element::scope() {
        return _scope;
    }

    compiler::element* case_element::expression() {
        return _expr;
    }

    void case_element::on_owned_elements(element_list_t& list) {
        if (_expr != nullptr)
            list.emplace_back(_expr);

        if (_scope != nullptr)
            list.emplace_back(_scope);
    }

};