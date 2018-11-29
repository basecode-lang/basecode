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
#include "namespace_element.h"

namespace basecode::compiler {

    block::block(
            compiler::module* module,
            block* parent_scope,
            element_type_t type) : element(module, parent_scope, type) {
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

    defer_stack_t& block::defer_stack() {
        return _defer_stack;
    }

    statement_list_t& block::statements() {
        return _statements;
    }

    identifier_map_t& block::identifiers() {
        return _identifiers;
    }

    bool block::on_emit(compiler::session& session) {
        for (auto stmt : _statements) {
            stmt->emit_labels(session);
            auto expr = stmt->expression();
            if (expr != nullptr
            &&  expr->element_type() == element_type_t::defer)
                continue;
            auto result = stmt->emit(session);
            if (!result)
                return false;
        }

        defer_stack_t working_stack = _defer_stack;
        while (!working_stack.empty()) {
            auto deferred = working_stack.top();
            auto result = deferred->emit(session);
            if (!result)
                return false;
            working_stack.pop();
        }

        return !session.result().is_failed();
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

};