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
#include <common/defer.h>
#include <compiler/session.h>
#include <compiler/element_builder.h>
#include "block.h"
#include "import.h"
#include "attribute.h"
#include "statement.h"
#include "identifier.h"
#include "assignment.h"
#include "declaration.h"
#include "defer_element.h"

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

    compiler::element* block::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        auto copy = session.builder().make_block(new_scope->module(), new_scope);

        return copy;
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

    reference_map_t& block::references() {
        return _references;
    }

    statement_list_t& block::statements() {
        return _statements;
    }

    identifier_map_t& block::identifiers() {
        return _identifiers;
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

    void block::add_expression_to_scope(compiler::element* e) {
        switch (e->element_type()) {
            case element_type_t::defer: {
                _defer_stack.push(dynamic_cast<compiler::defer_element*>(e));
                break;
            }
            case element_type_t::import_e: {
                _imports.emplace_back(dynamic_cast<compiler::import*>(e));
                break;
            }
            case element_type_t::attribute: {
                auto& attrs = attributes();
                attrs.add(dynamic_cast<compiler::attribute*>(e));
                break;
            }
            case element_type_t::statement: {
                auto statement = dynamic_cast<compiler::statement*>(e);
                _statements.emplace_back(statement);
                auto expr = statement->expression();
                if (expr != nullptr)
                    add_expression_to_scope(expr);
                break;
            }
            default:
                break;
        }
    }

}