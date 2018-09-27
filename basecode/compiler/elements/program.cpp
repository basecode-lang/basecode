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
#include <compiler/variable.h>
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
#include "declaration.h"
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
#include "type_reference.h"
#include "binary_operator.h"
#include "integer_literal.h"
#include "namespace_element.h"
#include "array_constructor.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    program::program() : element(nullptr, nullptr, element_type_t::program) {
    }

    program::~program() {
    }

    bool program::emit_section_variable(
            compiler::session& session,
            compiler::element* e,
            vm::instruction_block* instruction_block) {
        auto& assembler = session.assembler();

        switch (e->element_type()) {
            case element_type_t::array_constructor: {
                auto array_constructor = dynamic_cast<compiler::array_constructor*>(e);
                instruction_block->blank_line();
                instruction_block->align(4);
                auto var_label = assembler.make_label(array_constructor->label_name());
                instruction_block->label(var_label);
                // XXX: emit data
                break;
            }
            case element_type_t::identifier: {
                auto var = dynamic_cast<compiler::identifier*>(e);
                auto var_type = var->type_ref()->type();
                auto init = var->initializer();

                instruction_block->blank_line();

                auto type_alignment = static_cast<uint8_t>(var_type->alignment());
                if (type_alignment > 1)
                    instruction_block->align(type_alignment);

                instruction_block->comment(
                    fmt::format("identifier type: {}", var->type_ref()->name()),
                    0);
                auto var_label = assembler.make_label(var->symbol()->name());
                instruction_block->label(var_label);
//                session.allocate_variable(
//                    var_label->name(),
//                    var_type,
//                    identifier_usage_t::heap);

                switch (var_type->element_type()) {
                    case element_type_t::bool_type: {
                        bool value = false;
                        var->as_bool(value);

                        if (init == nullptr)
                            instruction_block->reserve_byte(1);
                        else
                            instruction_block->bytes({static_cast<uint8_t>(value ? 1 : 0)});
                        break;
                    }
                    case element_type_t::rune_type: {
                        common::rune_t value = common::rune_invalid;
                        var->as_rune(value);

                        if (init == nullptr)
                            instruction_block->reserve_byte(4);
                        else
                            instruction_block->dwords({static_cast<uint32_t>(value)});
                        break;
                    }
                    case element_type_t::pointer_type: {
                        if (init == nullptr)
                            instruction_block->reserve_qword(1);
                        else
                            instruction_block->qwords({0});
                        break;
                    }
                    case element_type_t::numeric_type: {
                        uint64_t value = 0;
                        if (var_type->number_class() == type_number_class_t::integer) {
                            var->as_integer(value);
                        } else {
                            double temp = 0;
                            if (var->as_float(temp)) {
                                vm::register_value_alias_t alias;
                                alias.qwf = temp;
                                value = alias.qw;
                            }
                        }

                        auto symbol_type = vm::integer_symbol_type_for_size(var_type->size_in_bytes());
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
                    case element_type_t::any_type:
                    case element_type_t::type_info:
                    case element_type_t::array_type:
                    case element_type_t::string_type:
                    case element_type_t::composite_type: {
                        instruction_block->reserve_byte(var_type->size_in_bytes());
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

        return true;
    }

    compiler::block* program::block() {
        return _block;
    }

    void program::block(compiler::block* value) {
        _block = value;
    }

    void program::initialize_variable_sections() {
        _vars_by_section.clear();
        _vars_by_section.insert(std::make_pair(vm::section_t::bss,     element_list_t()));
        _vars_by_section.insert(std::make_pair(vm::section_t::ro_data, element_list_t()));
        _vars_by_section.insert(std::make_pair(vm::section_t::data,    element_list_t()));
        _vars_by_section.insert(std::make_pair(vm::section_t::text,    element_list_t()));
    }

    bool program::on_emit(compiler::session& session) {
        _string_type = session.scope_manager().find_type(qualified_symbol_t {
            .name = "string"
        });

        intern_string_literals(session);
        initialize_variable_sections();

        if (!emit_bootstrap_block(session))
            return false;

        if (!emit_type_info(session))
            return false;

        if (!session.emit_interned_strings())
            return false;

        if (!emit_sections(session))
            return false;

        if (!emit_initializers(session))
            return false;

        if (!emit_implicit_blocks(session))
            return false;

        if (!emit_finalizers(session))
            return false;

        return true;
    }

    bool program::emit_sections(compiler::session& session) {
        if (!group_identifiers_by_section(session))
            return false;

        auto& assembler = session.assembler();
        auto instruction_block = assembler.make_basic_block();

        for (const auto& section : _vars_by_section) {
            instruction_block->blank_line();
            instruction_block->section(section.first);

            for (auto e : section.second)
                emit_section_variable(session, e, instruction_block);
        }

        return true;
    }

    bool program::emit_type_info(compiler::session& session) {
        auto& assembler = session.assembler();

        auto type_info_block = assembler.make_basic_block();
        type_info_block->blank_line();
        type_info_block->section(vm::section_t::ro_data);

        assembler.push_block(type_info_block);
        defer({
            assembler.pop_block();
        });

        std::unordered_map<common::id_t, compiler::type*> used_types {};

        auto declarations = session.elements().find_by_type(element_type_t::declaration);
        for (auto d : declarations) {
            auto decl = dynamic_cast<compiler::declaration*>(d);
            auto decl_type = decl->identifier()->type_ref()->type();
            if (decl_type == nullptr) {
                // XXX: this is an error!
                return false;
            }
            if (used_types.count(decl_type->id()) > 0)
                continue;
            used_types.insert(std::make_pair(decl_type->id(), decl_type));
        }

        for (const auto& kvp : used_types) {
            type_info_block->blank_line();
            type_info_block->align(4);
            auto label_name = fmt::format(
                "_ti_name_lit_{}",
                kvp.second->symbol()->name());
            type_info_block->string(
                assembler.make_label(label_name),
                assembler.make_label(label_name + "_data"),
                kvp.second->name());
        }

        type_info_block->blank_line();
        type_info_block->align(8);
        type_info_block->label(assembler.make_label("_ti_array"));
        type_info_block->qwords({used_types.size()});
        for (const auto& kvp : used_types) {
            kvp.second->emit_type_info(session);
        }

        return true;
    }

    bool program::emit_finalizers(compiler::session& session) {
        auto& assembler = session.assembler();

        auto finalizer_block = assembler.make_basic_block();
        finalizer_block->blank_line();
        finalizer_block->align(vm::instruction_t::alignment);
        finalizer_block->label(assembler.make_label("_finalizer"));

        assembler.push_block(finalizer_block);
        defer({
            finalizer_block->move_fp_to_sp();
            finalizer_block->exit();
            assembler.pop_block();
        });

        for (const auto& section : _vars_by_section) {
            for (auto e : section.second) {
                if (e->element_type() == element_type_t::identifier) {
                    auto var = dynamic_cast<compiler::identifier*>(e);
                    auto var_type = var->type_ref()->type();
                    if (!var_type->emit_finalizer(session, var))
                        return false;
                }
            }
        }

        return true;
    }

    bool program::emit_initializers(compiler::session& session) {
        auto& assembler = session.assembler();

        auto initializer_block = assembler.make_basic_block();
        initializer_block->blank_line();
        initializer_block->align(vm::instruction_t::alignment);
        initializer_block->label(assembler.make_label("_initializer"));
        initializer_block->move_sp_to_fp();

        assembler.push_block(initializer_block);
        defer({
            assembler.pop_block();
        });

        for (const auto& section : _vars_by_section) {
            for (auto e : section.second) {
                if (e->element_type() == element_type_t::identifier) {
                    auto var = dynamic_cast<compiler::identifier*>(e);
                    auto var_type = var->type_ref()->type();
                    if (!var_type->emit_initializer(session, var))
                        return false;
                }
            }
        }

        return true;
    }

    bool program::emit_bootstrap_block(compiler::session& session) {
        auto& assembler = session.assembler();

        auto instruction_block = assembler.make_basic_block();
        instruction_block->jump_direct(assembler.make_label_ref("_initializer"));

        return true;
    }

    bool program::emit_implicit_blocks(compiler::session& session) {
        auto& assembler = session.assembler();

        block_list_t implicit_blocks {};
        auto module_blocks = session.elements().find_by_type(element_type_t::module_block);
        for (auto block : module_blocks) {
            implicit_blocks.emplace_back(dynamic_cast<compiler::block*>(block));
        }

        for (auto block : implicit_blocks) {
            auto implicit_block = assembler.make_basic_block();
            implicit_block->blank_line();

            auto parent_element = block->parent_element();
            switch (parent_element->element_type()) {
                case element_type_t::namespace_e: {
                    auto parent_ns = dynamic_cast<compiler::namespace_element*>(parent_element);
                    implicit_block->comment(
                        fmt::format("namespace: {}", parent_ns->name()),
                        0);
                    break;
                }
                case element_type_t::module: {
                    auto parent_module = dynamic_cast<compiler::module*>(parent_element);
                    implicit_block->comment(
                        fmt::format("module: {}", parent_module->source_file()->path().string()),
                        0);
                    break;
                }
                default:
                    break;
            }

            implicit_block->label(assembler.make_label(block->label_name()));

            assembler.push_block(implicit_block);
            block->emit(session);
            assembler.pop_block();
        }

        return true;
    }

    bool program::emit_procedure_types(compiler::session& session) {
        auto& assembler = session.assembler();
        procedure_type_list_t proc_list {};

        auto procedure_types = session.elements().find_by_type(element_type_t::proc_type);
        for (auto p : procedure_types) {
            auto procedure_type = dynamic_cast<compiler::procedure_type*>(p);
            if (!procedure_type->instances().empty()) {
                proc_list.emplace_back(procedure_type);
            }
        }

        for (auto procedure_type : proc_list) {
            auto proc_type_block = assembler.make_basic_block();
            assembler.push_block(proc_type_block);
            procedure_type->emit(session);
            assembler.pop_block();
        }

        return true;
    }

    element_list_t* program::variable_section(vm::section_t section) {
        auto it = _vars_by_section.find(section);
        if (it == _vars_by_section.end()) {
            return nullptr;
        }
        return &it->second;
    }

    void program::intern_string_literals(compiler::session& session) {
        auto literals = session.elements().find_by_type(element_type_t::string_literal);
        for (auto literal : literals)
            session.intern_string(dynamic_cast<compiler::string_literal*>(literal));
    }

    bool program::group_identifiers_by_section(compiler::session& session) {
        auto& scope_manager = session.scope_manager();

        auto ro_list = variable_section(vm::section_t::ro_data);
        auto data_list = variable_section(vm::section_t::data);

        auto identifiers = session.elements().find_by_type(element_type_t::identifier);
        for (auto identifier : identifiers) {
            auto var = dynamic_cast<compiler::identifier*>(identifier);
            if (scope_manager.within_procedure_scope(var->parent_scope()))
                continue;

            auto var_parent = var->parent_element();
            if (var_parent != nullptr && var_parent->is_parent_element(element_type_t::field))
                continue;

            auto var_type = var->type_ref()->type();
            if (var_type == nullptr) {
                // XXX: this is an error!
                return false;
            }

            auto init = var->initializer();
            if (init != nullptr
                &&  (init->expression()->element_type() == element_type_t::type_reference
                     || init->expression()->element_type() == element_type_t::proc_type)) {
                continue;
            }

            if (var_type->element_type() == element_type_t::namespace_type)
                continue;

            if (var->is_constant()) {
                ro_list->emplace_back(var);
            } else {
                data_list->emplace_back(var);
            }
        }

        return true;
    }

};