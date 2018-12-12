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

#include <fmt/format.h>
#include <compiler/session.h>
#include <vm/instruction_block.h>
#include "if_element.h"

namespace basecode::compiler {

    if_element::if_element(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* predicate,
            compiler::element* true_branch,
            compiler::element* false_branch,
            bool is_else_if) : element(module, parent_scope, element_type_t::if_e),
                               _is_else_if(is_else_if),
                               _predicate(predicate),
                               _true_branch(true_branch),
                               _false_branch(false_branch) {
    }

    bool if_element::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        auto begin_label_name = fmt::format("{}_begin", label_name());
        auto true_label_name = fmt::format("{}_true", label_name());
        auto false_label_name = fmt::format("{}_false", label_name());
        auto end_label_name = fmt::format("{}_end", label_name());

        vm::instruction_operand_t result_operand;
        if (!vm::instruction_operand_t::allocate(
                assembler,
                result_operand,
                vm::op_sizes::byte,
                vm::register_type_t::integer)) {
            return false;
        }
        result.operands.emplace_back(result_operand);

        block->label(assembler.make_label(begin_label_name));

        emit_context_t if_context {};
        emit_result_t predicate_result {};
        _predicate->emit(session, if_context, predicate_result);

        block->bz(
            predicate_result.operands.back(),
            vm::instruction_operand_t(assembler.make_label_ref(false_label_name)));

        block->label(assembler.make_label(true_label_name));
        emit_result_t true_result {};
        _true_branch->emit(session, if_context, true_result);
        if (!block->is_current_instruction(vm::op_codes::jmp))
            block->jump_direct(assembler.make_label_ref(end_label_name));

        block->label(assembler.make_label(false_label_name));
        if (_false_branch != nullptr) {
            emit_result_t false_result {};
            _false_branch->emit(session, if_context, false_result);
        } else {
            block->nop();
        }

        block->label(assembler.make_label(end_label_name));

        return true;
    }

    bool if_element::is_else_if() const {
        return _is_else_if;
    }

    compiler::element* if_element::predicate() {
        return _predicate;
    }

    compiler::element* if_element::true_branch() {
        return _true_branch;
    }

    compiler::element* if_element::false_branch() {
        return _false_branch;
    }

    void if_element::predicate(compiler::element* value) {
        _predicate = value;
    }

    void if_element::on_owned_elements(element_list_t& list) {
        if (_predicate != nullptr)
            list.emplace_back(_predicate);
        if (_true_branch != nullptr)
            list.emplace_back(_true_branch);
        if (_false_branch != nullptr)
            list.emplace_back(_false_branch);
    }

};