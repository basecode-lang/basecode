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
#include <vm/assembler.h>
#include <common/defer.h>
#include <compiler/session.h>
#include <vm/instruction_block.h>
#include "block.h"
#include "import.h"
#include "module.h"
#include "statement.h"
#include "initializer.h"
#include "numeric_type.h"
#include "defer_element.h"
#include "symbol_element.h"
#include "procedure_type.h"
#include "string_literal.h"
#include "type_reference.h"
#include "namespace_element.h"

namespace basecode::compiler {

    block::block(
            compiler::module* module,
            block* parent_scope,
            element_type_t type) : element(module, parent_scope, type),
                                   _stack_frame(parent_scope != nullptr ? &parent_scope->stack_frame() : nullptr) {
    }

    bool block::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        if (!begin_stack_frame(session))
            return false;

        for (size_t index = 0; index < _statements.size(); ++index) {
            auto stmt = _statements[index];

            stmt->emit_labels(session);
            auto expr = stmt->expression();
            if (expr != nullptr
            &&  expr->element_type() == element_type_t::defer)
                continue;

            if (context.flow_control != nullptr) {
                compiler::element* prev = nullptr;
                compiler::element* next = nullptr;

                if (index > 0)
                    prev = _statements[index - 1];
                if (index < _statements.size() - 1)
                    next = _statements[index + 1];

                auto& values_map = context.flow_control->values;
                values_map[next_element] = next;
                values_map[previous_element] = prev;
            }

            emit_result_t stmt_result(session.assembler());
            auto success = stmt->emit(session, context, stmt_result);
            if (!success)
                return false;
        }

        defer_stack_t working_stack = _defer_stack;
        while (!working_stack.empty()) {
            auto deferred = working_stack.top();
            auto success = deferred->emit(session, context, result);
            if (!success)
                return false;
            working_stack.pop();
        }

        end_stack_frame(session);

        return !session.result().is_failed();
    }

    type_map_t& block::types() {
        return _types;
    }

    block_list_t& block::blocks() {
        return _blocks;
    }

    import_list_t& block::imports() {
        return _imports;
    }

    bool block::has_stack_frame() const {
        return is_parent_element(element_type_t::proc_type)
               || is_parent_element(element_type_t::proc_instance)
               || is_parent_element(element_type_t::block);
    }

    defer_stack_t& block::defer_stack() {
        return _defer_stack;
    }

    statement_list_t& block::statements() {
        return _statements;
    }

    identifier_map_t& block::identifiers() {
        return _identifiers;
    }

    compiler::stack_frame& block::stack_frame() {
        return _stack_frame;
    }

    void block::on_owned_elements(element_list_t& list) {
        for (auto element : _types.as_list())
            list.emplace_back(element);

        for (auto element : _blocks)
            list.emplace_back(element);

        for (auto element : _statements)
            list.emplace_back(element);

        for (auto element : _identifiers.as_list())
            list.emplace_back(element);

        for (auto element : _imports)
            list.emplace_back(element);
    }

    bool block::end_stack_frame(compiler::session& session) {
        if (!has_stack_frame())
            return true;

        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        for (auto var : _locals) {
            auto var_type = var->type_ref()->type();

            variable_handle_t temp_var {};
            if (!session.variable(var, temp_var))
                return false;

            if (!var_type->emit_finalizer(session, temp_var.get()))
                return false;
        }

        block->move(
            vm::instruction_operand_t::sp(),
            vm::instruction_operand_t::fp());
        block->pop(vm::instruction_operand_t::fp());

        return true;
    }

    bool block::begin_stack_frame(compiler::session& session) {
        if (!has_stack_frame())
            return true;

        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        block->push(vm::instruction_operand_t::fp());
        block->move(
            vm::instruction_operand_t::fp(),
            vm::instruction_operand_t::sp());

        auto& scope_manager = session.scope_manager();

        scope_manager.visit_blocks(
            session.result(),
            [&](compiler::block* scope) {
                if (scope->is_parent_element(element_type_t::proc_type))
                    return true;

                for (auto var : scope->identifiers().as_list()) {
                    auto type = var->type_ref()->type();
                    if (type->is_proc_type())
                        continue;

                    auto entry = _stack_frame.add(
                        stack_frame_entry_type_t::local,
                        var->symbol()->name(),
                        type->size_in_bytes());
                    var->stack_frame_entry(entry);

                    _locals.emplace_back(var);
                }

                return true;
            },
            this);

        auto locals_size = _stack_frame.type_size_in_bytes(stack_frame_entry_type_t::local);
        if (locals_size > 0) {
            block->sub(
                vm::instruction_operand_t::sp(),
                vm::instruction_operand_t::sp(),
                vm::instruction_operand_t(static_cast<uint64_t>(locals_size), vm::op_sizes::dword));
        }

        for (auto var : _locals) {
            auto var_type = var->type_ref()->type();

            variable_handle_t temp_var {};
            if (!session.variable(var, temp_var))
                return false;

            if (!var_type->emit_initializer(session, temp_var.get()))
                return false;
        }

        return true;
    }

};