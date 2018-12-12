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
#include <compiler/elements.h>
#include <compiler/variable.h>
#include <vm/instruction_block.h>

namespace basecode::compiler {

    program::program() : element(nullptr, nullptr, element_type_t::program) {
    }

    program::~program() {
    }

    bool program::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        _string_type = session
            .scope_manager()
            .find_type(qualified_symbol_t("string"));

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

        if (!emit_procedure_types(session))
            return false;

        if (!emit_start_block(session))
            return false;

        if (!emit_initializers(session))
            return false;

        if (!emit_implicit_blocks(session))
            return false;

        if (!emit_finalizers(session))
            return false;

        return emit_end_block(session);
    }

    bool program::emit_section_variable(
            compiler::session& session,
            compiler::element* e,
            vm::instruction_block* instruction_block) {
        auto& assembler = session.assembler();

        switch (e->element_type()) {
            case element_type_t::type_literal: {
                auto type_literal = dynamic_cast<compiler::type_literal*>(e);
                instruction_block->blank_line();
                instruction_block->align(4);
                auto var_label = assembler.make_label(type_literal->label_name());
                instruction_block->label(var_label);
                // XXX: emit data
                break;
            }
            case element_type_t::identifier: {
                auto var = dynamic_cast<compiler::identifier*>(e);

                if (!session.allocate_address_register(var->id()))
                    return false;

                auto var_type = var->type_ref()->type();
                auto init = var->initializer();

                instruction_block->blank_line();

                auto type_alignment = static_cast<uint8_t>(var_type->alignment());
                if (type_alignment > 1)
                    instruction_block->align(type_alignment);

                instruction_block->comment(fmt::format(
                    "identifier type: {}",
                    var->type_ref()->name()));
                auto var_label = assembler.make_label(var->label_name());
                instruction_block->label(var_label);

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
                        auto symbol_type = vm::integer_symbol_type_for_size(var_type->size_in_bytes());

                        if (var_type->number_class() == type_number_class_t::integer) {
                            var->as_integer(value);
                        } else {
                            double temp = 0;
                            if (var->as_float(temp)) {
                                vm::register_value_alias_t alias {};
                                if (symbol_type == vm::symbol_type_t::u32)
                                    alias.dwf = static_cast<float>(temp);
                                else
                                    alias.qwf = temp;
                                value = alias.qw;
                            }
                        }

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
                    case element_type_t::map_type:
                    case element_type_t::type_info:
                    case element_type_t::array_type:
                    case element_type_t::tuple_type:
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

    bool program::emit_end_block(compiler::session& session) {
        auto& assembler = session.assembler();

        auto end_block = assembler.make_basic_block();
        end_block->blank_line();
        end_block->align(vm::instruction_t::alignment);
        end_block->label(assembler.make_label("_end"));
        assembler.push_block(end_block);
        defer(assembler.pop_block());

        end_block->exit();

        return true;
    }

    bool program::emit_type_info(compiler::session& session) {
        auto& assembler = session.assembler();

        auto type_info_block = assembler.make_basic_block();
        type_info_block->blank_line();
        type_info_block->section(vm::section_t::ro_data);

        assembler.push_block(type_info_block);
        defer(assembler.pop_block());

        std::unordered_map<common::id_t, compiler::type*> used_types {};

        auto declarations = session.elements().find_by_type(element_type_t::declaration);
        for (auto d : declarations) {
            auto decl = dynamic_cast<compiler::declaration*>(d);
            auto decl_type = decl->identifier()->type_ref()->type();
            if (decl_type == nullptr) {
                // XXX: this is an error!
                return false;
            }
            if (decl_type->element_type() == element_type_t::generic_type)
                continue;
            if (used_types.count(decl_type->id()) > 0)
                continue;
            used_types.insert(std::make_pair(decl_type->id(), decl_type));
        }

        for (const auto& kvp : used_types) {
            type_info_block->blank_line();
            type_info_block->align(4);
            type_info_block->string(
                assembler.make_label(compiler::type::make_literal_label_name(kvp.second)),
                assembler.make_label(compiler::type::make_literal_data_label_name(kvp.second)),
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
        defer(assembler.pop_block());

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

    bool program::emit_start_block(compiler::session& session) {
        auto& assembler = session.assembler();

        auto start_block = assembler.make_basic_block();
        start_block->blank_line();
        start_block->align(vm::instruction_t::alignment);
        start_block->label(assembler.make_label("_start"));
        assembler.push_block(start_block);
        defer(assembler.pop_block());

        auto address_registers = session.address_registers();
        for (auto kvp : address_registers) {
            auto var = dynamic_cast<compiler::identifier*>(session.elements().find(kvp.first));
            start_block->comment(
                var->symbol()->name(),
                vm::comment_location_t::after_instruction);
            start_block->move(
                vm::instruction_operand_t(kvp.second),
                vm::instruction_operand_t(assembler.make_label_ref(var->label_name())));
        }

        return true;
    }

    bool program::emit_initializers(compiler::session& session) {
        auto& assembler = session.assembler();

        auto initializer_block = assembler.make_basic_block();
        initializer_block->blank_line();
        initializer_block->align(vm::instruction_t::alignment);
        initializer_block->label(assembler.make_label("_initializer"));

        assembler.push_block(initializer_block);
        defer(assembler.pop_block());

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
        instruction_block->jump_direct(assembler.make_label_ref("_start"));

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
                    implicit_block->comment(fmt::format(
                        "namespace: {}",
                        parent_ns->name()));
                    break;
                }
                case element_type_t::module: {
                    auto parent_module = dynamic_cast<compiler::module*>(parent_element);
                    implicit_block->comment(fmt::format(
                        "module: {}",
                        parent_module->source_file()->path().string()));
                    break;
                }
                default:
                    break;
            }

            implicit_block->label(assembler.make_label(block->label_name()));

            assembler.push_block(implicit_block);
            emit_context_t context {};
            emit_result_t result {};
            block->emit(session, context, result);
            assembler.pop_block();
        }

        return true;
    }

    bool program::emit_procedure_types(compiler::session& session) {
        auto& assembler = session.assembler();
        procedure_instance_set_t proc_instance_set {};

        auto proc_calls = session.elements().find_by_type(element_type_t::proc_call);
        for (auto call : proc_calls) {
            auto proc_call = dynamic_cast<compiler::procedure_call*>(call);
            auto type = proc_call->reference()->identifier()->type_ref()->type();

            auto procedure_type = dynamic_cast<compiler::procedure_type*>(type);
            if (procedure_type->is_foreign())
                continue;

            auto instance = procedure_type->instance_for(session, proc_call);
            if (instance != nullptr)
                proc_instance_set.insert(instance);
        }

        if (session.result().is_failed())
            return false;

        for (auto instance : proc_instance_set) {
            auto proc_type_block = assembler.make_basic_block();
            assembler.push_block(proc_type_block);

            emit_context_t context {};
            emit_result_t result {};
            instance->procedure_type()->emit(session, context, result);

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

        std::set<common::id_t> processed_identifiers {};

        auto identifier_refs = session.elements().find_by_type(element_type_t::identifier_reference);
        for (auto r : identifier_refs) {
            auto ref = dynamic_cast<compiler::identifier_reference*>(r);
            auto var = ref->identifier();
            if (processed_identifiers.count(var->id()) > 0)
                continue;

            processed_identifiers.insert(var->id());

            if (scope_manager.within_local_scope(var->parent_scope()))
                continue;

            auto var_parent = var->parent_element();
            if (var_parent != nullptr && var_parent->is_parent_element(element_type_t::field))
                continue;

            auto var_type = var->type_ref()->type();
            if (var_type == nullptr) {
                // XXX: this is an error!
                return false;
            }

            if (var_type->element_type() == element_type_t::generic_type)
                continue;

            auto init = var->initializer();
            if (init != nullptr) {
                switch (init->expression()->element_type()) {
                    case element_type_t::directive: {
                        auto directive = dynamic_cast<compiler::directive*>(init->expression());
                        if (directive->name() == "type")
                            continue;
                    }
                    case element_type_t::proc_type:
                    case element_type_t::composite_type:
                    case element_type_t::type_reference:
                    case element_type_t::module_reference:
                        continue;
                    default:
                        break;
                }
            }

            if (var_type->element_type() == element_type_t::namespace_type
            ||  var_type->element_type() == element_type_t::module_reference) {
                continue;
            }

            if (var->is_constant()) {
                ro_list->emplace_back(var);
            } else {
                data_list->emplace_back(var);
            }
        }

        return true;
    }

};