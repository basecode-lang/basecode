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
#include <compiler/session.h>
#include <vm/instruction_block.h>
#include "elements.h"
#include "byte_code_emitter.h"

namespace basecode::compiler {

    byte_code_emitter::byte_code_emitter(compiler::session& session) : _session(session) {
    }

    ///////////////////////////////////////////////////////////////////////////

    vm::instruction_block* byte_code_emitter::pop_block() {
        if (_block_stack.empty())
            return nullptr;

        auto top = _block_stack.top();
        _block_stack.pop();
        return top;
    }

    vm::instruction_block* byte_code_emitter::current_block() {
        if (_block_stack.empty())
            return nullptr;

        return _block_stack.top();
    }

    void byte_code_emitter::push_block(vm::instruction_block* block) {
        _block_stack.push(block);
    }

    ///////////////////////////////////////////////////////////////////////////

    bool byte_code_emitter::allocate_register(
            vm::instruction_operand_t& op,
            vm::op_sizes size,
            vm::register_type_t type) {
        vm::register_t reg {};
        reg.type = type;
        reg.size = size;

        if (!_session.assembler().allocate_reg(reg))
            return false;

        op = vm::instruction_operand_t(reg);

        return true;
    }

    ///////////////////////////////////////////////////////////////////////////

    bool byte_code_emitter::emit() {
        identifier_by_section_t vars {};
        vars.insert(std::make_pair(vm::section_t::bss,     element_list_t()));
        vars.insert(std::make_pair(vm::section_t::ro_data, element_list_t()));
        vars.insert(std::make_pair(vm::section_t::data,    element_list_t()));
        vars.insert(std::make_pair(vm::section_t::text,    element_list_t()));

        intern_string_literals();

        if (!emit_bootstrap_block())
            return false;

        if (!emit_type_table())
            return false;

        if (!emit_interned_string_table())
            return false;

        if (!emit_section_tables(vars))
            return false;

        if (!emit_procedure_types())
            return false;

        if (!emit_start_block())
            return false;

        if (!emit_initializers(vars))
            return false;

        if (!emit_implicit_blocks())
            return false;

        if (!emit_finalizers(vars))
            return false;

        return emit_end_block();
    }

    bool byte_code_emitter::emit_element(
            vm::instruction_block* block,
            compiler::element* e,
            emit_result_t& result) {
        auto& assembler = _session.assembler();
        e->infer_type(_session, result.type_result);

        switch (e->element_type()) {
            case element_type_t::cast: {
                break;
            }
            case element_type_t::if_e: {
                break;
            }
            case element_type_t::with: {
                break;
            }
            case element_type_t::for_e: {
                break;
            }
            case element_type_t::label: {
                block->blank_line();
                block->label(assembler.make_label(e->label_name()));
                break;
            }
            case element_type_t::block: {
                auto cblock = dynamic_cast<compiler::block*>(e);
                identifier_list_t locals {};

                if (!begin_stack_frame(block, cblock, locals))
                    return false;

                const auto& statements = cblock->statements();
                for (size_t index = 0; index < statements.size(); ++index) {
                    auto stmt = statements[index];

                    for (auto label : stmt->labels()) {
                        emit_result_t label_result {};
                        if (!emit_element(block, label, label_result))
                            return false;
                    }

                    auto expr = stmt->expression();
                    if (expr != nullptr
                    &&  expr->element_type() == element_type_t::defer)
                        continue;

//                    if (context.flow_control != nullptr) {
//                        compiler::element* prev = nullptr;
//                        compiler::element* next = nullptr;
//
//                        if (index > 0)
//                            prev = statements[index - 1];
//                        if (index < statements.size() - 1)
//                            next = statements[index + 1];
//
//                        auto& values_map = context.flow_control->values;
//                        values_map[next_element] = next;
//                        values_map[previous_element] = prev;
//                    }

                    emit_result_t stmt_result;
                    if (!emit_element(block, stmt, stmt_result))
                        return false;
                }

                auto working_stack = cblock->defer_stack();
                while (!working_stack.empty()) {
                    auto deferred = working_stack.top();

                    emit_result_t defer_result {};
                    if (!emit_element(block, deferred, defer_result))
                        return false;
                    working_stack.pop();
                }

                end_stack_frame(block, cblock, locals);
                break;
            }
            case element_type_t::field: {
                break;
            }
            case element_type_t::defer: {
                break;
            }
            case element_type_t::symbol: {
                break;
            }
            case element_type_t::module: {
                break;
            }
            case element_type_t::case_e: {
                break;
            }
            case element_type_t::break_e: {
                break;
            }
            case element_type_t::comment: {
                break;
            }
            case element_type_t::program: {
                break;
            }
            case element_type_t::while_e: {
                break;
            }
            case element_type_t::element: {
                break;
            }
            case element_type_t::return_e: {
                break;
            }
            case element_type_t::import_e: {
                break;
            }
            case element_type_t::switch_e: {
                break;
            }
            case element_type_t::raw_block: {
                break;
            }
            case element_type_t::intrinsic: {
                auto intrinsic = dynamic_cast<compiler::intrinsic*>(e);
                const auto& name = intrinsic->name();

                auto args = intrinsic->arguments()->elements();
                if (name == "address_of") {
                    auto arg = args[0];

                    emit_result_t arg_result {};
                    if (!emit_element(block, arg, arg_result))
                        return false;

                    vm::instruction_operand_t result_operand;
                    if (!allocate_register(result_operand, vm::op_sizes::qword, vm::register_type_t::integer))
                        return false;

                    result.operands.emplace_back(result_operand);
                } else if (name == "alloc") {
                    auto arg = args[0];

                    emit_result_t arg_result {};
                    if (!emit_element(block, arg, arg_result))
                        return false;

                    vm::instruction_operand_t result_operand;
                    if (!allocate_register(result_operand, vm::op_sizes::qword, vm::register_type_t::integer))
                        return false;

                    result.operands.emplace_back(result_operand);

                    block->alloc(
                        vm::op_sizes::byte,
                        result_operand,
                        arg_result.operands.back());
                } else if (name == "free") {
                    auto arg = args[0];

                    emit_result_t arg_result {};
                    if (!emit_element(block, arg, arg_result))
                        return false;

                    block->free(arg_result.operands.back());
                } else if (name == "fill") {
                    auto dest_arg = args[0];
                    auto value_arg = args[1];
                    auto length_arg = args[2];

                    emit_result_t dest_arg_result {};
                    if (!emit_element(block, dest_arg, dest_arg_result))
                        return false;

                    emit_result_t value_arg_result {};
                    if (!emit_element(block, value_arg, value_arg_result))
                        return false;

                    emit_result_t length_arg_result {};
                    if (!emit_element(block, length_arg, length_arg_result))
                        return false;

                    block->fill(
                        vm::op_sizes::byte,
                        dest_arg_result.operands.back(),
                        value_arg_result.operands.back(),
                        length_arg_result.operands.back());
                } else if (name == "copy") {
                    auto dest_arg = args[0];
                    auto src_arg = args[1];
                    auto size_arg = args[2];

                    emit_result_t dest_arg_result {};
                    if (!emit_element(block, dest_arg, dest_arg_result))
                        return false;

                    emit_result_t src_arg_result {};
                    if (!emit_element(block, src_arg, src_arg_result))
                        return false;

                    emit_result_t size_arg_result {};
                    if (!emit_element(block, size_arg, size_arg_result))
                        return false;

                    block->copy(
                        vm::op_sizes::byte,
                        dest_arg_result.operands.back(),
                        src_arg_result.operands.back(),
                        size_arg_result.operands.back());
                }
                break;
            }
            case element_type_t::directive: {
                break;
            }
            case element_type_t::attribute: {
                break;
            }
            case element_type_t::statement: {
                auto statement = dynamic_cast<compiler::statement*>(e);
                auto expr = statement->expression();
                if (expr != nullptr) {
                    emit_result_t expr_result {};
                    if (!emit_element(block, expr, expr_result))
                        return false;
                }
                break;
            }
            case element_type_t::proc_call: {
                break;
            }
            case element_type_t::transmute: {
                break;
            }
            case element_type_t::continue_e: {
                break;
            }
            case element_type_t::identifier: {
                break;
            }
            case element_type_t::expression: {
                auto expr = dynamic_cast<compiler::expression*>(e);
                auto root = expr->root();
                if (root != nullptr)
                    return emit_element(block, root, result);
                break;
            }
            case element_type_t::assignment: {
                break;
            }
            case element_type_t::declaration: {
                break;
            }
            case element_type_t::namespace_e: {
                break;
            }
            case element_type_t::initializer: {
                auto init = dynamic_cast<compiler::initializer*>(e);
                auto expr = init->expression();
                if (expr != nullptr)
                    return emit_element(block, expr, result);
                break;
            }
            case element_type_t::fallthrough: {
                break;
            }
            case element_type_t::nil_literal: {
                result.operands.emplace_back(vm::instruction_operand_t(
                    static_cast<uint64_t>(0),
                    vm::op_sizes::qword));
                break;
            }
            case element_type_t::type_literal: {
                break;
            }
            case element_type_t::float_literal: {
                auto float_literal = dynamic_cast<compiler::float_literal*>(e);
                auto value = float_literal->value();
                auto is_float = numeric_type::narrow_to_value(value) == "f32";
                if (is_float) {
                    auto temp_value = static_cast<float>(value);
                    result.operands.emplace_back(vm::instruction_operand_t(temp_value));
                } else {
                    result.operands.emplace_back(vm::instruction_operand_t(value));
                }
                break;
            }
            case element_type_t::string_literal: {
                result.operands.emplace_back(vm::instruction_operand_t(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::label,
                    interned_string_data_label(e->id()))));
                break;
            }
            case element_type_t::boolean_literal: {
                auto bool_literal = dynamic_cast<compiler::boolean_literal*>(e);
                result.operands.emplace_back(vm::instruction_operand_t(
                    static_cast<uint64_t>(bool_literal->value() ? 1 : 0),
                    vm::op_sizes::byte));
                break;
            }
            case element_type_t::integer_literal: {
                auto integer_literal = dynamic_cast<compiler::integer_literal*>(e);
                infer_type_result_t type_result {};
                if (!e->infer_type(_session, type_result))
                    return false;
                result.operands.emplace_back(vm::instruction_operand_t(
                    integer_literal->value(),
                    vm::op_size_for_byte_size(type_result.inferred_type->size_in_bytes())));
                break;
            }
            case element_type_t::character_literal: {
                auto char_literal = dynamic_cast<compiler::character_literal*>(e);
                result.operands.emplace_back(vm::instruction_operand_t(
                    static_cast<int64_t>(char_literal->rune()),
                    vm::op_sizes::dword));
                break;
            }
            case element_type_t::uninitialized_literal: {
                break;
            }
            case element_type_t::rune_type:
            case element_type_t::proc_type:
            case element_type_t::bool_type:
            case element_type_t::tuple_type:
            case element_type_t::array_type:
            case element_type_t::module_type:
            case element_type_t::unknown_type:
            case element_type_t::numeric_type:
            case element_type_t::pointer_type:
            case element_type_t::generic_type:
            case element_type_t::namespace_type:
            case element_type_t::composite_type: {
                break;
            }
            case element_type_t::argument_pair: {
                break;
            }
            case element_type_t::argument_list: {
                break;
            }
            case element_type_t::proc_instance: {
                auto proc_instance = dynamic_cast<compiler::procedure_instance*>(e);
                auto procedure_type = proc_instance->procedure_type();
                if (procedure_type->is_foreign())
                    return true;

                if (!emit_procedure_prologue(block, procedure_type))
                    return false;

                emit_result_t scope_result {};
                if (!emit_element(block, proc_instance->scope(), scope_result))
                    return false;

                if (!emit_procedure_epilogue(block, procedure_type))
                    return false;

                break;
            }
            case element_type_t::assembly_label: {
                auto label = dynamic_cast<compiler::assembly_label*>(e);
                auto name = label->reference()->identifier()->symbol()->name();
                if (assembler.has_local(name)) {
                    result.operands.emplace_back(_session.assembler().make_named_ref(
                        vm::assembler_named_ref_type_t::local,
                        name));
                } else {
                    result.operands.emplace_back(_session.assembler().make_named_ref(
                        vm::assembler_named_ref_type_t::label,
                        name));
                }
                break;
            }
            case element_type_t::unary_operator: {
                auto unary_op = dynamic_cast<compiler::unary_operator*>(e);
                auto op_type = unary_op->operator_type();

                emit_result_t rhs_emit_result {};
                if (!emit_element(block, unary_op->rhs(), rhs_emit_result))
                    return false;

                auto is_composite_type = rhs_emit_result.type_result.inferred_type->is_composite_type();
                auto size = vm::op_size_for_byte_size(result.type_result.inferred_type->size_in_bytes());
                if (op_type == operator_type_t::pointer_dereference
                &&  !is_composite_type) {
                    auto pointer_type = dynamic_cast<compiler::pointer_type*>(result.type_result.inferred_type);
                    size = vm::op_size_for_byte_size(pointer_type->base_type_ref()->type()->size_in_bytes());
                }

                vm::instruction_operand_t result_operand;
                if (!allocate_register(result_operand, size, vm::register_type_t::integer))
                    return false;
                result.operands.emplace_back(result_operand);

                switch (op_type) {
                    case operator_type_t::negate: {
                        block->comment("unary_op: negate", vm::comment_location_t::after_instruction);
                        block->neg(
                            result_operand,
                            rhs_emit_result.operands.back());
                        break;
                    }
                    case operator_type_t::binary_not: {
                        block->comment("unary_op: binary not", vm::comment_location_t::after_instruction);
                        block->not_op(
                            result_operand,
                            rhs_emit_result.operands.back());
                        break;
                    }
                    case operator_type_t::logical_not: {
                        block->comment("unary_op: logical not", vm::comment_location_t::after_instruction);
                        block->cmp(
                            result_operand.size(),
                            rhs_emit_result.operands.back(),
                            vm::instruction_operand_t(static_cast<uint64_t>(1), vm::op_sizes::byte));
                        block->setnz(result_operand);
                        break;
                    }
                    case operator_type_t::pointer_dereference: {
                        if (!is_composite_type) {
                            block->comment("unary_op: deref", vm::comment_location_t::after_instruction);
                            block->load(
                                result_operand,
                                rhs_emit_result.operands.back());
                        } else {
                            result.operands.push_back(rhs_emit_result.operands.back());
                        }
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            case element_type_t::type_reference: {
                break;
            }
            case element_type_t::binary_operator: {
                break;
            }
            case element_type_t::spread_operator: {
                break;
            }
            case element_type_t::label_reference: {
                break;
            }
            case element_type_t::module_reference: {
                break;
            }
            case element_type_t::unknown_identifier: {
                break;
            }
            case element_type_t::identifier_reference: {
                break;
            }
            case element_type_t::assembly_literal_label: {
                auto label = dynamic_cast<compiler::assembly_literal_label*>(e);
                result.operands.emplace_back(vm::instruction_operand_t(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::label,
                    label->name())));
                break;
            }
        }
        return true;
    }

    bool byte_code_emitter::emit_type_info(
            vm::instruction_block* block,
            compiler::type* type) {
        if (type == nullptr)
            return false;

        if (type->element_type() == element_type_t::generic_type
        ||  type->element_type() == element_type_t::unknown_type) {
            return true;
        }

        auto& assembler = _session.assembler();

        auto type_name = type->name();
        auto type_name_len = static_cast<uint32_t>(type_name.length());
        auto label_name = type::make_info_label_name(type);

        block->blank_line();
        block->comment(fmt::format("type: {}", type_name), 0);
        block->label(assembler.make_label(label_name));

        block->dwords({type_name_len});
        block->dwords({type_name_len});
        block->qwords({assembler.make_named_ref(
            vm::assembler_named_ref_type_t::label,
            type::make_literal_data_label_name(type))});

        return true;
    }

    bool byte_code_emitter::emit_type_table() {
        auto& assembler = _session.assembler();

        auto type_info_block = assembler.make_basic_block();
        type_info_block->section(vm::section_t::ro_data);

        auto used_types = _session.used_types();
        for (auto type : used_types) {
            type_info_block->blank_line();
            type_info_block->align(4);
            type_info_block->string(
                assembler.make_label(compiler::type::make_literal_label_name(type)),
                assembler.make_label(compiler::type::make_literal_data_label_name(type)),
                type->name());
        }

        type_info_block->blank_line();
        type_info_block->align(8);
        type_info_block->label(assembler.make_label("_ti_array"));
        type_info_block->qwords({used_types.size()});
        for (auto type : used_types) {
            emit_type_info(type_info_block, type);
        }

        return true;
    }

    bool byte_code_emitter::emit_bootstrap_block() {
        auto& assembler = _session.assembler();

        auto block = assembler.make_basic_block();
        block->jump_direct(vm::instruction_operand_t(assembler.make_named_ref(
            vm::assembler_named_ref_type_t::label,
            "_start")));

        return true;
    }

    void byte_code_emitter::intern_string_literals() {
        auto literals = _session
            .elements()
            .find_by_type<compiler::string_literal>(element_type_t::string_literal);
        for (auto literal : literals) {
            if (literal->is_parent_type_one_of({
                    element_type_t::attribute,
                    element_type_t::directive,
                    element_type_t::module_reference})) {
                continue;
            }

            _session.intern_string(literal);
        }
    }

    bool byte_code_emitter::emit_interned_string_table() {
        auto& assembler = _session.assembler();

        auto block = assembler.make_basic_block();
        block->comment("interned string literals", 0);
        block->section(vm::section_t::ro_data);

        auto interned_strings = _session.interned_strings();
        for (const auto& kvp : interned_strings) {
            block->blank_line();
            block->align(4);
            block->comment(
                fmt::format("\"{}\"", kvp.first),
                0);

            std::string escaped {};
            if (!compiler::string_literal::escape(kvp.first, escaped)) {
                _session.error(
                    nullptr,
                    "X000",
                    fmt::format("invalid escape sequence: {}", kvp.first),
                    {});
                return false;
            }

            block->string(
                assembler.make_label(fmt::format("_intern_str_lit_{}", kvp.second)),
                assembler.make_label(fmt::format("_intern_str_lit_{}_data", kvp.second)),
                escaped);
        }

        return true;
    }

    bool byte_code_emitter::emit_section_tables(identifier_by_section_t& vars) {
        if (!group_identifiers(vars))
            return false;

        auto& assembler = _session.assembler();
        auto block = assembler.make_basic_block();

        for (const auto& section : vars) {
            block->blank_line();
            block->section(section.first);

            for (auto e : section.second)
                emit_section_variable(block, e);
        }

        return true;
    }

    element_list_t* byte_code_emitter::variable_section(
            identifier_by_section_t& groups,
            vm::section_t section) {
        auto it = groups.find(section);
        if (it == groups.end()) {
            return nullptr;
        }
        return &it->second;
    }

    bool byte_code_emitter::emit_end_block() {
        auto& assembler = _session.assembler();

        auto end_block = assembler.make_basic_block();
        end_block->align(vm::instruction_t::alignment);
        end_block->label(assembler.make_label("_end"));
        end_block->exit();

        return true;
    }

    bool byte_code_emitter::emit_start_block() {
        auto& assembler = _session.assembler();

        auto start_block = assembler.make_basic_block();
        start_block->align(vm::instruction_t::alignment);
        start_block->label(assembler.make_label("_start"));

        start_block->move(
            vm::instruction_operand_t::fp(),
            vm::instruction_operand_t::sp());

        auto address_registers = _session.address_registers();
        for (auto kvp : address_registers) {
            auto var = dynamic_cast<compiler::identifier*>(_session.elements().find(kvp.first));
            start_block->comment(
                var->label_name(),
                vm::comment_location_t::after_instruction);
            start_block->move(
                vm::instruction_operand_t(kvp.second),
                vm::instruction_operand_t(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::label,
                    var->label_name())));
        }

        return true;
    }

    bool byte_code_emitter::emit_implicit_blocks() {
        auto& assembler = _session.assembler();

        block_list_t implicit_blocks {};
        auto module_refs = _session
            .elements()
            .find_by_type<compiler::module_reference>(element_type_t::module_reference);
        for (auto mod_ref : module_refs) {
            auto block = mod_ref->reference()->scope();
            // XXX: how can we check a block to determine if it will emit byte code?
            //      if it won't, then don't add it here
            implicit_blocks.emplace_back(block);
        }
        implicit_blocks.emplace_back(_session.program().module()->scope());

        for (auto block : implicit_blocks) {
            auto implicit_block = assembler.make_basic_block();

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

            push_block(implicit_block);
            emit_result_t result {};
            if (!emit_element(implicit_block, block, result))
                return false;
            pop_block();
        }

        return true;
    }

    bool byte_code_emitter::emit_procedure_types() {
        procedure_instance_set_t proc_instance_set {};

        auto proc_calls = _session
            .elements()
            .find_by_type<compiler::procedure_call>(element_type_t::proc_call);
        for (auto proc_call : proc_calls) {
            if (proc_call->is_foreign())
                continue;

            auto proc_type = proc_call->procedure_type();
            if (proc_type == nullptr)
                return false;

            auto instance = proc_type->instance_for(_session, proc_call);
            if (instance != nullptr)
                proc_instance_set.insert(instance);
        }

        if (_session.result().is_failed())
            return false;

        auto& assembler = _session.assembler();
        for (auto instance : proc_instance_set) {
            auto basic_block = assembler.make_basic_block();
            push_block(basic_block);

            emit_result_t result {};
            if (!emit_element(basic_block, instance, result))
                return false;

            pop_block();
        }

        return true;
    }

    bool byte_code_emitter::emit_finalizers(identifier_by_section_t& vars) {
        auto& assembler = _session.assembler();

        auto block = assembler.make_basic_block();
        block->align(vm::instruction_t::alignment);
        block->label(assembler.make_label("_finalizer"));

        push_block(block);
        defer(pop_block());

        std::vector<compiler::identifier*> to_finalize {};
        for (const auto& section : vars) {
            for (compiler::element* e : section.second) {
                auto var = dynamic_cast<compiler::identifier*>(e);
                if (var == nullptr)
                    continue;

                if (!var->type_ref()->is_composite_type()) {
                    continue;
                }

                auto local_type = number_class_to_local_type(var->type_ref()->type()->number_class());
                block->local(local_type, var->symbol()->name());
                to_finalize.emplace_back(var);
            }
        }

        block->blank_line();

        for (auto var : to_finalize) {
            block->move(
                vm::instruction_operand_t(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::local,
                    var->symbol()->name())),
                vm::instruction_operand_t(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::label,
                    var->label_name())));
        }

        for (auto var : to_finalize) {
            emit_finalizer(block, var);
        }

        return true;
    }

    bool byte_code_emitter::group_identifiers(identifier_by_section_t& vars) {
        auto& scope_manager = _session.scope_manager();

        auto ro_list = variable_section(vars, vm::section_t::ro_data);
        auto data_list = variable_section(vars, vm::section_t::data);

        std::set<common::id_t> processed_identifiers {};

        auto identifier_refs = _session
            .elements()
            .find_by_type<compiler::identifier_reference>(element_type_t::identifier_reference);
        for (auto ref : identifier_refs) {
            auto var = ref->identifier();
            if (processed_identifiers.count(var->id()) > 0)
                continue;

            processed_identifiers.insert(var->id());

            if (scope_manager.within_local_scope(var->parent_scope()))
                continue;

            auto var_parent = var->parent_element();
            if (var_parent != nullptr
            &&  var_parent->is_parent_type_one_of({element_type_t::field})) {
                continue;
            }

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

    bool byte_code_emitter::emit_initializers(identifier_by_section_t& vars) {
        auto& assembler = _session.assembler();

        auto block = assembler.make_basic_block();
        block->align(vm::instruction_t::alignment);
        block->label(assembler.make_label("_initializer"));

        push_block(block);
        defer(pop_block());

        std::vector<compiler::identifier*> to_init {};
        for (const auto& section : vars) {
            for (compiler::element* e : section.second) {
                auto var = dynamic_cast<compiler::identifier*>(e);
                if (var == nullptr)
                    continue;

                if (var->is_constant()
                && !var->type_ref()->is_composite_type()) {
                    continue;
                }

                auto init = var->initializer();
                if (init != nullptr) {
                    if (init->expression()->element_type() == element_type_t::uninitialized_literal)
                        continue;
                }

                auto local_type = number_class_to_local_type(var->type_ref()->type()->number_class());
                block->local(local_type, var->symbol()->name());
                to_init.emplace_back(var);
            }
        }

        block->blank_line();

        for (auto var : to_init) {
            block->move(
                vm::instruction_operand_t(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::local,
                    var->symbol()->name())),
                vm::instruction_operand_t(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::label,
                    var->label_name())));
        }

        for (auto var : to_init) {
            emit_initializer(block, var);
        }

        return true;
    }

    bool byte_code_emitter::emit_section_variable(
            vm::instruction_block* block,
            compiler::element* e) {
        auto& assembler = _session.assembler();

        switch (e->element_type()) {
            case element_type_t::type_literal: {
                auto type_literal = dynamic_cast<compiler::type_literal*>(e);
                block->blank_line();
                block->align(4);
                auto var_label = assembler.make_label(type_literal->label_name());
                block->label(var_label);
                // XXX: emit data
                break;
            }
            case element_type_t::identifier: {
                auto var = dynamic_cast<compiler::identifier*>(e);

                auto var_type = var->type_ref()->type();
                auto init = var->initializer();

                block->blank_line();

                auto type_alignment = static_cast<uint8_t>(var_type->alignment());
                if (type_alignment > 1)
                    block->align(type_alignment);

                block->comment(fmt::format(
                    "identifier type: {}",
                    var->type_ref()->name()));
                auto var_label = assembler.make_label(var->label_name());
                block->label(var_label);

                switch (var_type->element_type()) {
                    case element_type_t::bool_type: {
                        bool value = false;
                        var->as_bool(value);

                        if (init == nullptr)
                            block->reserve_byte(1);
                        else
                            block->bytes({static_cast<uint8_t>(value ? 1 : 0)});
                        break;
                    }
                    case element_type_t::rune_type: {
                        common::rune_t value = common::rune_invalid;
                        var->as_rune(value);

                        if (init == nullptr)
                            block->reserve_byte(4);
                        else
                            block->dwords({static_cast<uint32_t>(value)});
                        break;
                    }
                    case element_type_t::pointer_type: {
                        if (init == nullptr)
                            block->reserve_qword(1);
                        else
                            block->qwords({0});
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
                                    block->reserve_byte(1);
                                else
                                    block->bytes({static_cast<uint8_t>(value)});
                                break;
                            case vm::symbol_type_t::u16:
                                if (init == nullptr)
                                    block->reserve_word(1);
                                else
                                    block->words({static_cast<uint16_t>(value)});
                                break;
                            case vm::symbol_type_t::f32:
                            case vm::symbol_type_t::u32:
                                if (init == nullptr)
                                    block->reserve_dword(1);
                                else
                                    block->dwords({static_cast<uint32_t>(value)});
                                break;
                            case vm::symbol_type_t::f64:
                            case vm::symbol_type_t::u64:
                                if (init == nullptr)
                                    block->reserve_qword(1);
                                else
                                    block->qwords({value});
                                break;
                            case vm::symbol_type_t::bytes:
                                break;
                            default:
                                break;
                        }
                        break;
                    }
                    case element_type_t::array_type:
                    case element_type_t::tuple_type:
                    case element_type_t::composite_type: {
                        block->reserve_byte(var_type->size_in_bytes());
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

    bool byte_code_emitter::emit_primitive_initializer(
            vm::instruction_block* block,
            const vm::instruction_operand_t& base_local,
            compiler::identifier* var,
            int64_t offset) {
        auto var_type = var->type_ref()->type();
        auto init = var->initializer();

        uint64_t default_value = var_type->element_type() == element_type_t::rune_type ?
                                 common::rune_invalid :
                                 0;
        vm::instruction_operand_t value(
            default_value,
            vm::op_size_for_byte_size(var_type->size_in_bytes()));
        vm::instruction_operand_t* value_ptr = &value;

        emit_result_t result {};
        if (init != nullptr) {
            if (!emit_element(block, init, result))
                return false;
            value_ptr = &result.operands.back();
        }

        block->comment(
            fmt::format("initializer: {}: {}", var->symbol()->name(), var_type->name()),
            vm::comment_location_t::after_instruction);
        block->store(
            base_local,
            *value_ptr,
            vm::instruction_operand_t::offset(offset));
        return true;
    }

    bool byte_code_emitter::emit_finalizer(
            vm::instruction_block* block,
            compiler::identifier* var) {
        auto var_type = var->type_ref()->type();

        block->comment(
            fmt::format("finalizer: {}: {}", var->symbol()->name(), var_type->name()),
            4);

        return true;
    }

    bool byte_code_emitter::emit_initializer(
            vm::instruction_block* block,
            compiler::identifier* var) {
        vm::instruction_operand_t base_local(_session.assembler().make_named_ref(
            vm::assembler_named_ref_type_t::local,
            var->symbol()->name()));

        std::vector<compiler::identifier*> list {};
        list.emplace_back(var);

        uint64_t offset = 0;

        while (!list.empty()) {
            auto next_var = list.front();
            list.erase(std::begin(list));

            auto var_type = next_var->type_ref()->type();

            switch (var_type->element_type()) {
                case element_type_t::rune_type:
                case element_type_t::bool_type:
                case element_type_t::numeric_type:
                case element_type_t::pointer_type: {
                    if (!emit_primitive_initializer(block, base_local, next_var, offset))
                        return false;
                    offset += var_type->size_in_bytes();
                    break;
                }
                case element_type_t::tuple_type:
                case element_type_t::composite_type: {
                    auto composite_type = dynamic_cast<compiler::composite_type*>(var_type);
                    switch (composite_type->type()) {
                        case composite_types_t::enum_type: {
                            if (!emit_primitive_initializer(block, base_local, next_var, offset))
                                return false;
                            offset += var_type->size_in_bytes();
                            break;
                        }
                        case composite_types_t::union_type: {
                            // XXX: intentional no-op
                            break;
                        }
                        case composite_types_t::struct_type: {
                            auto field_list = composite_type->fields().as_list();
                            size_t index = 0;
                            for (auto fld: field_list) {
                                list.emplace(std::begin(list) + index, fld->identifier());
                                index++;
                            }
                            offset = common::align(offset, composite_type->alignment());
                            break;
                        }
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }

        return true;
    }

    bool byte_code_emitter::end_stack_frame(
            vm::instruction_block* basic_block,
            compiler::block* block,
            const identifier_list_t& locals) {
        if (!block->has_stack_frame())
            return true;

        identifier_list_t to_finalize {};
        for (compiler::element* e : locals) {
            auto var = dynamic_cast<compiler::identifier*>(e);
            if (var == nullptr)
                continue;

            if (!var->type_ref()->is_composite_type()) {
                continue;
            }

            to_finalize.emplace_back(var);
        }

        if (!basic_block->is_current_instruction(vm::op_codes::rts)) {
            basic_block->move(
                vm::instruction_operand_t::sp(),
                vm::instruction_operand_t::fp());
            basic_block->pop(vm::instruction_operand_t::fp());
        }

        return true;
    }

    bool byte_code_emitter::begin_stack_frame(
            vm::instruction_block* basic_block,
            compiler::block* block,
            identifier_list_t& locals) {
        if (!block->has_stack_frame())
            return true;

        auto& assembler = _session.assembler();
        auto& scope_manager = _session.scope_manager();

        basic_block->push(vm::instruction_operand_t::fp());
        basic_block->move(
            vm::instruction_operand_t::fp(),
            vm::instruction_operand_t::sp());

        scope_manager.visit_blocks(
            _session.result(),
            [&](compiler::block* scope) {
                if (scope->is_parent_type_one_of({element_type_t::proc_type}))
                    return true;

                auto stack_frame = block->stack_frame();
                if (stack_frame == nullptr)
                    return true;

                for (auto var : scope->identifiers().as_list()) {
                    auto type = var->type_ref()->type();
                    if (type->is_proc_type())
                        continue;

                    auto entry = stack_frame->find(var->symbol()->name());
                    if (entry == nullptr) {
                        entry = stack_frame->add(
                            stack_frame_entry_type_t::local,
                            var->symbol()->name(),
                            type->size_in_bytes());
                        locals.emplace_back(var);
                    }
                    var->stack_frame_entry(entry);
                    basic_block->local(vm::local_type_t::integer, entry->name(), entry->offset());
                }

                return true;
            },
            block);

        auto locals_size = block->stack_frame()->type_size_in_bytes(stack_frame_entry_type_t::local);
        if (locals_size > 0) {
            basic_block->sub(
                vm::instruction_operand_t::sp(),
                vm::instruction_operand_t::sp(),
                vm::instruction_operand_t(static_cast<uint64_t>(locals_size), vm::op_sizes::dword));
        }

        for (auto var : locals) {
            const auto& name = var->symbol()->name();
            basic_block->move(
                vm::instruction_operand_t(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::local,
                    name)),
                vm::instruction_operand_t::fp(),
                vm::instruction_operand_t(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::offset,
                    name)));
        }

        identifier_list_t to_init {};
        for (auto var : locals) {
            if (var->is_constant()
            && !var->type_ref()->is_composite_type()) {
                continue;
            }

            auto init = var->initializer();
            if (init != nullptr) {
                if (init->expression()->element_type() == element_type_t::uninitialized_literal)
                    continue;
            }

            to_init.emplace_back(var);
        }

        return true;
    }

    bool byte_code_emitter::emit_procedure_epilogue(
            vm::instruction_block* block,
            compiler::procedure_type* proc_type) {
        if (proc_type->is_foreign())
            return true;

        if (!proc_type->has_return()) {
            block->rts();
        }

        return true;
    }

    bool byte_code_emitter::emit_procedure_prologue(
            vm::instruction_block* block,
            compiler::procedure_type* proc_type) {
        if (proc_type->is_foreign())
            return true;

        auto& assembler = _session.assembler();

        auto procedure_label = proc_type->symbol()->name();
        auto parent_init = proc_type->parent_element_as<compiler::initializer>();
        if (parent_init != nullptr) {
            auto parent_var = parent_init->parent_element_as<compiler::identifier>();
            if (parent_var != nullptr) {
                procedure_label = parent_var->label_name();
            }
        }

        block->align(vm::instruction_t::alignment);
        block->label(assembler.make_label(procedure_label));

        auto frame = proc_type->scope()->stack_frame();

        auto& stack_offsets = frame->offsets();
        stack_offsets.locals = 8;

        auto return_type = proc_type->return_type();
        if (return_type != nullptr) {
            auto entry = frame->add(
                stack_frame_entry_type_t::return_slot,
                return_type->identifier()->symbol()->name(),
                8);
            return_type->identifier()->stack_frame_entry(entry);

            stack_offsets.return_slot = 16;
            stack_offsets.parameters = 24;
        } else {
            stack_offsets.parameters = 16;
        }

        auto fields = proc_type->parameters().as_list();
        for (auto fld : fields) {
            auto var = fld->identifier();
            auto type = var->type_ref()->type();
            // XXX: if we change procedure_call to
            //      sub.qw sp, sp, {size}
            //
            //      and then store.x sp, {value}, offset
            //      we can use truer sizes within
            //      the 8-byte aligned stack block.
            //
            auto entry = frame->add(
                stack_frame_entry_type_t::parameter,
                var->symbol()->name(),
                common::align(type->size_in_bytes(), 8));
            var->stack_frame_entry(entry);
        }

        return true;
    }

    std::string byte_code_emitter::interned_string_data_label(common::id_t id) {
        common::id_t intern_id;
        _session.interned_strings().element_id_to_intern_id(id, intern_id);
        return fmt::format("_intern_str_lit_{}_data", intern_id);
    }

};
