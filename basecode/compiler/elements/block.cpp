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
#include "block.h"
#include "import.h"
#include "statement.h"
#include "identifier.h"
#include "assignment.h"
#include "declaration.h"

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

    bool block::has_statements() const {
        block_stack_t block_stack {};
        block_stack.push(const_cast<compiler::block*>(this));

        while (!block_stack.empty()) {
            auto current_block = block_stack.top();
            block_stack.pop();

            for (auto s : current_block->statements()) {
                auto expr = s->expression();
                if (expr == nullptr)
                    continue;
                switch (expr->element_type()) {
                    case element_type_t::import_e: {
                        continue;
                    }
                    case element_type_t::assignment: {
                        auto assignment = dynamic_cast<compiler::assignment*>(expr);
                        if (assignment != nullptr) {
                            for (auto e : assignment->expressions()) {
                                if (e->element_type() == element_type_t::binary_operator)
                                    return true;
                                else if (e->element_type() == element_type_t::declaration) {
                                    auto decl = dynamic_cast<compiler::declaration*>(e);
                                    if (decl != nullptr) {
                                        if (decl->is_constant())
                                            continue;
                                        if (decl->assignment() != nullptr)
                                            return true;
                                    }
                                }
                            }
                        }
                        break;
                    }
                    default: {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    defer_stack_t& block::defer_stack() {
        return _defer_stack;
    }

    bool block::has_stack_frame() const {
        return _has_stack_frame;
    }

    reference_map_t& block::references() {
        return _references;
    }

    statement_list_t& block::statements() {
        return _statements;
    }

    identifier_map_t& block::identifiers() {
        return _identifiers;
    }

    void block::has_stack_frame(bool value) {
        if (_has_stack_frame != value) {
            _has_stack_frame = value;
            for (auto var : _identifiers.as_list())
                var->usage(value ? identifier_usage_t::stack : identifier_usage_t::heap);
            for (auto child_block : _blocks)
                child_block->has_stack_frame(value);
        }
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

}