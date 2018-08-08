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
#include <vm/instruction_block.h>
#include "block.h"
#include "import.h"
#include "module.h"
#include "comment.h"
#include "statement.h"
#include "initializer.h"
#include "numeric_type.h"
#include "symbol_element.h"
#include "procedure_type.h"
#include "string_literal.h"
#include "namespace_element.h"

namespace basecode::compiler {

    block::block(
            block* parent_scope,
            element_type_t type) : element(parent_scope, type) {
    }

    bool block::on_emit(
            common::result& r,
            emit_context_t& context) {
        vm::instruction_block* instruction_block = nullptr;

        auto clean_up = false;
        defer({
            if (clean_up)
                context.assembler->pop_block();
        });

        switch (element_type()) {
            case element_type_t::block: {
                instruction_block = context.assembler->make_basic_block();
                instruction_block->memo();

                auto parent_ns = parent_element_as<compiler::namespace_element>();
                if (parent_ns != nullptr) {
                    instruction_block->current_entry()->comment(fmt::format(
                        "namespace: {}",
                        parent_ns->name()));
                }
                instruction_block->current_entry()->blank_lines(1);

                auto block_label = instruction_block->make_label(label_name());
                instruction_block
                    ->current_entry()
                    ->label(block_label);

                context.assembler->push_block(instruction_block);
                clean_up = true;
                break;
            }
            case element_type_t::module_block: {
                instruction_block = context.assembler->make_basic_block();
                instruction_block->memo();

                auto parent_module = parent_element_as<compiler::module>();
                if (parent_module != nullptr) {
                    instruction_block->current_entry()->comment(fmt::format(
                        "module: {}",
                        parent_module->source_file()->path().string()));
                    clean_up = !parent_module->is_root();
                }
                instruction_block->current_entry()->blank_lines(1);

                auto block_label = instruction_block->make_label(label_name());
                instruction_block
                    ->current_entry()
                    ->label(block_label);

                context.assembler->push_block(instruction_block);
                break;
            }
            case element_type_t::proc_type_block:
            case element_type_t::proc_instance_block: {
                break;
            }
            default: {
                return false;
            }
        }

        for (auto stmt : _statements)
            stmt->emit(r, context);

        return !r.is_failed();
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

    comment_list_t& block::comments() {
        return _comments;
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

        for (auto element : _comments)
            list.emplace_back(element);

        for (auto element : _statements)
            list.emplace_back(element);

        for (auto element : _identifiers.as_list())
            list.emplace_back(element);

        for (auto element : _imports)
            list.emplace_back(element);
    }

};