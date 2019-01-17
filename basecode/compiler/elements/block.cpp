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
                                   _stack_frame(parent_scope != nullptr ? &parent_scope->_stack_frame : nullptr) {
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

    void block::activate_stack_frame() {
        _stack_frame.active(true);
    }

    bool block::has_stack_frame() const {
        return _stack_frame.active();
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

    compiler::stack_frame* block::stack_frame() {
        return _stack_frame.active() ? &_stack_frame : nullptr;
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

    compiler::stack_frame* block::find_active_frame() {
        auto current = this;
        while (current != nullptr) {
            if (current->_stack_frame.active())
                return &current->_stack_frame;
            current = current->parent_scope();
        }
        return nullptr;
    }

    compiler::stack_frame_entry* block::find_active_frame_entry(const std::string& symbol) {
        auto current = this;
        while (current != nullptr) {
            if (current->_stack_frame.active()) {
                auto entry = current->_stack_frame.find(symbol);
                if (entry != nullptr)
                    return entry;
            }
            current = current->parent_scope();
        }
        return nullptr;
    }

};