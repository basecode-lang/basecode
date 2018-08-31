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

#include <vm/terp.h>
#include <fmt/format.h>
#include <vm/assembler.h>
#include <common/defer.h>
#include <common/bytes.h>
#include <compiler/session.h>
#include <vm/instruction_block.h>
#include "type.h"
#include "block.h"
#include "module.h"
#include "program.h"
#include "any_type.h"
#include "directive.h"
#include "bool_type.h"
#include "type_info.h"
#include "identifier.h"
#include "tuple_type.h"
#include "string_type.h"
#include "initializer.h"
#include "module_type.h"
#include "numeric_type.h"
#include "unknown_type.h"
#include "float_literal.h"
#include "symbol_element.h"
#include "string_literal.h"
#include "procedure_type.h"
#include "namespace_type.h"
#include "binary_operator.h"
#include "integer_literal.h"
#include "identifier_reference.h"


namespace basecode::compiler {

    program::program() : element(nullptr, nullptr, element_type_t::program) {
    }

    program::~program() {
    }

    compiler::block* program::block() {
        return _block;
    }

    void program::block(compiler::block* value) {
        _block = value;
    }

    bool program::on_emit(compiler::session& session) {
        auto& assembler = session.assembler();
        auto& scope_manager = session.scope_manager();

        auto instruction_block = assembler.make_basic_block();
        instruction_block->jump_direct(assembler.make_label_ref("_initializer"));

        std::map<vm::section_t, element_list_t> vars_by_section {};
        /* auto bss  = */vars_by_section.insert(std::make_pair(vm::section_t::bss,     element_list_t()));
        auto ro   = vars_by_section.insert(std::make_pair(vm::section_t::ro_data, element_list_t()));
        auto data = vars_by_section.insert(std::make_pair(vm::section_t::data,    element_list_t()));
        /* auto text = */vars_by_section.insert(std::make_pair(vm::section_t::text,    element_list_t()));

        auto identifiers = session.elements().find_by_type(element_type_t::identifier);
        for (auto identifier : identifiers) {
            auto var = dynamic_cast<compiler::identifier*>(identifier);
            if (scope_manager.within_procedure_scope(var->parent_scope()))
                continue;

            if (var->is_parent_element(element_type_t::field))
                continue;

            auto var_type = var->type();
            if (var_type == nullptr) {
                // XXX: this is an error!
                return false;
            }

            if (var_type->element_type() == element_type_t::namespace_type)
                continue;

            if (var->is_constant()) {
                auto& list = ro.first->second;
                list.emplace_back(var);
            } else {
                auto& list = data.first->second;
                list.emplace_back(var);
            }
        }

        auto& interned_strings = scope_manager.interned_string_literals();
        auto& ro_list = ro.first->second;
        for (const auto& it : interned_strings) {
            compiler::string_literal* str = it.second.front();
            if (!str->is_parent_element(element_type_t::argument_list))
                continue;
            ro_list.emplace_back(str);
        }

        for (const auto& section : vars_by_section) {
            instruction_block->blank_line();
            instruction_block->section(section.first);

            for (auto e : section.second) {
                switch (e->element_type()) {
                    case element_type_t::string_literal: {
                        auto string_literal = dynamic_cast<compiler::string_literal*>(e);
                        instruction_block->blank_line();
                        instruction_block->align(4);

                        auto it = interned_strings.find(string_literal->value());
                        if (it != interned_strings.end()) {
                            string_literal_list_t& str_list = it->second;
                            for (auto str : str_list) {
                                auto var_label = assembler.make_label(str->label_name());
                                instruction_block->label(var_label);

                                auto var = session.emit_context().allocate_variable(
                                    var_label->name(),
                                    session.scope_manager().find_type({.name = "string"}),
                                    identifier_usage_t::heap,
                                    nullptr);
                                if (var != nullptr)
                                    var->address_offset = 4;
                            }
                        }
                        instruction_block->comment(
                            fmt::format("\"{}\"", string_literal->value()),
                            session.emit_context().indent);
                        instruction_block->string(string_literal->escaped_value());

                        break;
                    }
                    case element_type_t::identifier: {
                        auto var = dynamic_cast<compiler::identifier*>(e);
                        auto init = var->initializer();

                        instruction_block->blank_line();

                        auto type_alignment = static_cast<uint8_t>(var->type()->alignment());
                        if (type_alignment > 1)
                            instruction_block->align(type_alignment);
                        auto var_label = assembler.make_label(var->symbol()->name());
                        instruction_block->label(var_label);
                        session.emit_context().allocate_variable(
                            var_label->name(),
                            var->type(),
                            identifier_usage_t::heap);

                        switch (var->type()->element_type()) {
                            case element_type_t::bool_type: {
                                bool value = false;
                                var->as_bool(value);

                                if (init == nullptr)
                                    instruction_block->reserve_byte(1);
                                else
                                    instruction_block->bytes({static_cast<uint8_t>(value ? 1 : 0)});
                                break;
                            }
                            case element_type_t::pointer_type: {
                                instruction_block->reserve_qword(1);
                                break;
                            }
                            case element_type_t::numeric_type: {
                                uint64_t value = 0;
                                if (var->type()->number_class() == type_number_class_t::integer) {
                                    var->as_integer(value);
                                } else {
                                    double temp = 0;
                                    if (var->as_float(temp)) {
                                        vm::register_value_alias_t alias;
                                        alias.qwf = temp;
                                        value = alias.qw;
                                    }
                                }

                                auto symbol_type = vm::integer_symbol_type_for_size(
                                    var->type()->size_in_bytes());
                                switch (symbol_type) {
                                    case vm::symbol_type_t::u8:
                                        if (init == nullptr)
                                            instruction_block->reserve_byte(1);
                                        else
                                            instruction_block->bytes({static_cast<uint8_t>(value)});
                                        break;
                                    case vm::symbol_type_t::u16:
                                        if (init == nullptr)
                                            instruction_block->reserve_word(1);
                                        else
                                            instruction_block->words({static_cast<uint16_t>(value)});
                                        break;
                                    case vm::symbol_type_t::f32:
                                    case vm::symbol_type_t::u32:
                                        if (init == nullptr)
                                            instruction_block->reserve_dword(1);
                                        else
                                            instruction_block->dwords({static_cast<uint32_t>(value)});
                                        break;
                                    case vm::symbol_type_t::f64:
                                    case vm::symbol_type_t::u64:
                                        if (init == nullptr)
                                            instruction_block->reserve_qword(1);
                                        else
                                            instruction_block->qwords({value});
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
                                    auto string_literal = dynamic_cast<compiler::string_literal*>(
                                        init->expression());
                                    if (string_literal != nullptr) {
                                        instruction_block->comment(
                                            fmt::format("\"{}\"", string_literal->value()),
                                            session.emit_context().indent);
                                        instruction_block->string(string_literal->value());
                                    }
                                }
                                break;
                            }
                            default: {
                                break;
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }

        assembler.push_block(instruction_block);

        auto procedure_types = session.elements().find_by_type(element_type_t::proc_type);
        procedure_type_list_t proc_list {};
        for (auto p : procedure_types) {
            auto procedure_type = dynamic_cast<compiler::procedure_type*>(p);
            if (procedure_type->parent_scope()->element_type() == element_type_t::proc_instance_block) {
                proc_list.emplace_back(procedure_type);
            }
        }

        for (auto p : procedure_types) {
            auto procedure_type = dynamic_cast<compiler::procedure_type*>(p);
            if (procedure_type->parent_scope()->element_type() != element_type_t::proc_instance_block) {
                proc_list.emplace_back(procedure_type);
            }
        }

        for (auto procedure_type : proc_list) {
            procedure_type->emit(session);
        }

        auto top_level_block = assembler.make_basic_block();
        top_level_block->blank_line();
        top_level_block->align(vm::instruction_t::alignment);
        top_level_block->label(assembler.make_label("_initializer"));

        block_list_t implicit_blocks {};
        auto module_blocks = session.elements().find_by_type(element_type_t::module_block);
        for (auto block : module_blocks) {
            implicit_blocks.emplace_back(dynamic_cast<compiler::block*>(block));
        }

        assembler.push_block(top_level_block);
        for (auto block : implicit_blocks)
            block->emit(session);

        auto finalizer_block = assembler.make_basic_block();
        finalizer_block->blank_line();
        finalizer_block->align(vm::instruction_t::alignment);
        finalizer_block->label(assembler.make_label("_finalizer"));
        finalizer_block->exit();

        assembler.pop_block();
        assembler.pop_block();

        return true;
    }

};