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
#include <vm/instruction_block.h>
#include "block.h"
#include "statement.h"
#include "initializer.h"
#include "numeric_type.h"
#include "procedure_type.h"

namespace basecode::compiler {

    block::block(
        block* parent_scope,
        element_type_t type) : element(parent_scope, type) {
    }

    bool block::on_emit(
            common::result& r,
            emit_context_t& context) {
        vm::instruction_block* instruction_block = nullptr;

        switch (element_type()) {
            case element_type_t::block:
                instruction_block = context.assembler->make_basic_block();
                instruction_block->make_label(label_name());
                context.assembler->push_block(instruction_block);
                break;
            case element_type_t::proc_type_block: {
                instruction_block = context.assembler->current_block();
                break;
            }
            case element_type_t::proc_instance_block: {
                instruction_block = context.assembler->current_block();
                break;
            }
            default:
                return false;
        }

        for (auto var : _identifiers.as_list()) {
            if (context.assembler->in_procedure_scope())
                var->usage(identifier_usage_t::stack);

            auto init = var->initializer();
            if (init == nullptr)
                continue;

            auto procedure_type = init->procedure_type();
            if (procedure_type != nullptr) {
                context.push_procedure_type(var->name());
                procedure_type->emit(r, context);
                context.pop();
            }
        }

        for (auto stmt : _statements) {
            stmt->emit(r, context);
        }

        for (auto blk : _blocks) {
            blk->emit(r, context);
        }

        if (element_type() == element_type_t::block)
            context.assembler->pop_block();

        return !r.is_failed();
    }

    void block::add_symbols(
            common::result& r,
            vm::segment* segment,
            const identifier_list_t& list) {
        for (auto var : list) {
            switch (var->type()->element_type()) {
                case element_type_t::bool_type:
                case element_type_t::numeric_type: {
                    auto symbol = segment->symbol(
                        var->name(),
                        vm::integer_symbol_type_for_size(var->type()->size_in_bytes()));
                    uint64_t value;
                    if (var->as_integer(value)) {
                        symbol->value(value);
                    }
                    break;
                }
                case element_type_t::any_type:
                case element_type_t::type_info:
                case element_type_t::array_type:
                case element_type_t::string_type:
                case element_type_t::composite_type: {
                    auto size_in_bytes = var->type()->size_in_bytes();
                    auto symbol = segment->symbol(
                        var->name(),
                        vm::symbol_type_t::bytes,
                        size_in_bytes);
                    if (var->initializer() != nullptr)
                        symbol->pending_address_from_id(var->initializer()->id());
                    uint64_t value = 0;
                    symbol->value(value);
                    break;
                }
                default:
                    break;
            }
        }
    }

    bool block::define_data(
            common::result& r,
            string_set_t& interned_strings,
            vm::assembler& assembler) {
        // look for constant strings

        if (!_identifiers.empty()) {
            auto constant_init = _identifiers.constants(true);
            if (!constant_init.empty()) {
                auto section = assembler.segment(
                    fmt::format("rodata_{}", id()),
                    vm::segment_type_t::constant);
                section->initialized(true);
                add_symbols(r, section, constant_init);
            }

            auto constant_uninit = _identifiers.constants(false);
            if (!constant_uninit.empty()) {
                auto section = assembler.segment(
                    fmt::format("bss_{}", id()),
                    vm::segment_type_t::constant);
                add_symbols(r, section, constant_uninit);
            }

            auto global_init = _identifiers.globals(true);
            if (!global_init.empty()) {
                auto section = assembler.segment(
                    fmt::format("data_{}", id()),
                    vm::segment_type_t::data);
                add_symbols(r, section, global_init);
            }

            auto global_uninit = _identifiers.globals(false);
            if (!global_uninit.empty()) {
                auto section = assembler.segment(
                    fmt::format("bss_data_{}", id()),
                    vm::segment_type_t::data);
                add_symbols(r, section, global_uninit);
            }
        }

        return !r.is_failed();
    }

    type_map_t& block::types() {
        return _types;
    }

    block_list_t& block::blocks() {
        return _blocks;
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

};