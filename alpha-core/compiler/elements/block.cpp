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

        switch (element_type()) {
            case element_type_t::block: {
                instruction_block = context.assembler->make_basic_block();
                instruction_block->nop();

                auto parent_ns = parent_element_as<compiler::namespace_element>();
                if (parent_ns != nullptr) {
                    instruction_block
                        ->current_entry()
                        ->comment(fmt::format("namespace: {}", parent_ns->name()));
                }

                auto block_label = instruction_block->make_label(label_name());
                instruction_block
                    ->current_entry()
                    ->label(block_label);

                context.assembler->push_block(instruction_block);

                break;
            }
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
            if (var->type()->element_type() == element_type_t::namespace_type)
                continue;

            auto init = var->initializer();
            if (init != nullptr) {
                auto procedure_type = init->procedure_type();
                if (procedure_type != nullptr) {
                    procedure_type->emit(r, context);
                    continue;
                }
            }

            if (context.assembler->in_procedure_scope())
                var->usage(identifier_usage_t::stack);
            else {
                instruction_block->make_label(var->name());

                switch (var->type()->element_type()) {
                    case element_type_t::numeric_type: {
                        if (var->is_constant())
                            instruction_block->section(vm::section_t::ro_data);
                        else
                            instruction_block->section(vm::section_t::data);

                        uint64_t value = 0;
                        var->as_integer(value);

                        auto symbol_type = vm::integer_symbol_type_for_size(
                            var->type()->size_in_bytes());
                        switch (symbol_type) {
                            case vm::symbol_type_t::u8:
                                if (init == nullptr)
                                    instruction_block->reserve_byte(1);
                                else
                                    instruction_block->byte(static_cast<uint8_t>(value));
                                break;
                            case vm::symbol_type_t::u16:
                                if (init == nullptr)
                                    instruction_block->reserve_word(1);
                                else
                                    instruction_block->word(static_cast<uint16_t>(value));
                                break;
                            case vm::symbol_type_t::f32:
                            case vm::symbol_type_t::u32:
                                if (init == nullptr)
                                    instruction_block->reserve_dword(1);
                                else
                                    instruction_block->dword(static_cast<uint32_t>(value));
                                break;
                            case vm::symbol_type_t::f64:
                            case vm::symbol_type_t::u64:
                                if (init == nullptr)
                                    instruction_block->reserve_qword(1);
                                else
                                    instruction_block->qword(value);
                                break;
                            case vm::symbol_type_t::bytes:
                                break;
                            default:
                                break;
                        }
                        break;
                    }
                    case element_type_t::string_type: {
                        if (init != nullptr) {
                            instruction_block->section(vm::section_t::ro_data);
                            auto string_literal = dynamic_cast<compiler::string_literal*>(
                                init->expression());
                            instruction_block->string(string_literal->value());
                        }
                        break;
                    }
                    default: {
                        break;
                    }
                }
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