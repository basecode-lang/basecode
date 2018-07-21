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
#include <compiler/session.h>
#include <vm/instruction_block.h>
#include "type.h"
#include "cast.h"
#include "label.h"
#include "alias.h"
#include "import.h"
#include "module.h"
#include "comment.h"
#include "any_type.h"
#include "attribute.h"
#include "directive.h"
#include "statement.h"
#include "type_info.h"
#include "expression.h"
#include "identifier.h"
#include "if_element.h"
#include "array_type.h"
#include "tuple_type.h"
#include "initializer.h"
#include "string_type.h"
#include "numeric_type.h"
#include "unknown_type.h"
#include "argument_list.h"
#include "float_literal.h"
#include "string_literal.h"
#include "unary_operator.h"
#include "composite_type.h"
#include "procedure_type.h"
#include "return_element.h"
#include "procedure_call.h"
#include "namespace_type.h"
#include "symbol_element.h"
#include "boolean_literal.h"
#include "binary_operator.h"
#include "integer_literal.h"
#include "namespace_element.h"
#include "procedure_instance.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    program::program(
        vm::terp* terp,
        vm::assembler* assembler) : element(nullptr, element_type_t::program),
                                    _terp(terp),
                                    _assembler(assembler) {
    }

    program::~program() {
    }

    void program::disassemble(FILE* file) {
        auto root_block = _assembler->root_block();
        root_block->disassemble();
        if (file != nullptr)
            _assembler->listing().write(file);
    }

    element* program::evaluate(
            common::result& r,
            compiler::session& session,
            const syntax::ast_node_shared_ptr& node,
            element_type_t default_block_type) {
        if (node == nullptr)
            return nullptr;

        switch (node->type) {
            case syntax::ast_node_types_t::symbol: {
                return make_symbol_from_node(r, node);
            }
            case syntax::ast_node_types_t::attribute: {
                return make_attribute(
                    current_scope(),
                    node->token.value,
                    evaluate(r, session, node->lhs));
            }
            case syntax::ast_node_types_t::directive: {
                auto expression = evaluate(r, session, node->lhs);
                auto directive_element = make_directive(
                    current_scope(),
                    node->token.value,
                    expression);
                apply_attributes(r, session, directive_element, node);
                directive_element->evaluate(r, session, this);
                return directive_element;
            }
            case syntax::ast_node_types_t::module: {
                auto module_block = make_block(
                    _block,
                    element_type_t::module_block);
                // XXX: this isn't adding to the parent's block list
                push_scope(module_block);
                _top_level_stack.push(module_block);

                for (auto it = node->children.begin();
                     it != node->children.end();
                     ++it) {
                    add_expression_to_scope(
                        module_block,
                        evaluate(r, session, *it, default_block_type));
                }

                _top_level_stack.pop();

                return make_module(_block, pop_scope());
            }
            case syntax::ast_node_types_t::basic_block: {
                auto active_scope = push_new_block(default_block_type);

                for (auto it = node->children.begin();
                     it != node->children.end();
                     ++it) {
                    auto expr = evaluate(r, session, *it, default_block_type);
                    if (expr != nullptr)
                        add_expression_to_scope(active_scope, expr);
                }

                return pop_scope();
            }
            case syntax::ast_node_types_t::statement: {
                label_list_t labels {};

                if (node->lhs != nullptr) {
                    for (const auto& label : node->lhs->children) {
                        labels.push_back(make_label(
                            current_scope(),
                            label->token.value));
                    }
                }

                auto expr = evaluate(r, session, node->rhs);
                if (expr->element_type() == element_type_t::symbol) {
                    type_find_result_t find_type_result {};
                    find_identifier_type(
                        r,
                        find_type_result,
                        node->rhs->rhs);
                    if (find_type_result.type == nullptr) {
                        r.add_message(
                            "P002",
                            fmt::format(
                                "unknown type '{}'.",
                                find_type_result.type_name.name),
                            true);
                        return nullptr;
                    }
                    add_identifier_to_scope(
                        r,
                        session,
                        dynamic_cast<compiler::symbol_element*>(expr),
                        find_type_result,
                        nullptr);
                }

                return make_statement(current_scope(), labels, expr);
            }
            case syntax::ast_node_types_t::expression: {
                return make_expression(
                    current_scope(),
                    evaluate(r, session, node->lhs));
            }
            case syntax::ast_node_types_t::assignment: {
                const auto& assignment_target_list = node->lhs;

                identifier_list_t list {};
                for (const auto& symbol_node : assignment_target_list->children) {
                    qualified_symbol_t qualified_symbol {};
                    make_qualified_symbol(qualified_symbol, symbol_node);
                    auto existing_identifier = find_identifier(qualified_symbol);
                    if (existing_identifier != nullptr) {
                        auto binary_op = make_binary_operator(
                            current_scope(),
                            operator_type_t::assignment,
                            existing_identifier,
                            evaluate(r, session, node->rhs));
                        apply_attributes(r, session, binary_op, node);
                        return binary_op;
                    } else {
                        auto symbol = dynamic_cast<compiler::symbol_element*>(evaluate(
                            r,
                            session,
                            symbol_node));
                        type_find_result_t find_type_result {};
                        find_identifier_type(
                            r,
                            find_type_result,
                            symbol_node->rhs);
                        auto new_identifier = add_identifier_to_scope(
                            r,
                            session,
                            symbol,
                            find_type_result,
                            node);
                        list.push_back(new_identifier);
                    }
                }

                // XXX: handle proper multi-assignment
                auto result = list.front();
                return result;
            }
            case syntax::ast_node_types_t::line_comment: {
                return make_comment(
                    current_scope(),
                    comment_type_t::line,
                    node->token.value);
            }
            case syntax::ast_node_types_t::block_comment: {
                return make_comment(
                    current_scope(),
                    comment_type_t::block,
                    node->token.value);
            }
            case syntax::ast_node_types_t::string_literal: {
                return make_string(
                    current_scope(),
                    node->token.value);
            }
            case syntax::ast_node_types_t::number_literal: {
                switch (node->token.number_type) {
                    case syntax::number_types_t::integer: {
                        uint64_t value;
                        if (node->token.parse(value) == syntax::conversion_result_t::success)
                            return make_integer(current_scope(), value);
                        // XXX: need to handle conversion failures
                    }
                    case syntax::number_types_t::floating_point: {
                        double value;
                        if (node->token.parse(value) == syntax::conversion_result_t::success)
                            return make_float(current_scope(), value);
                        // XXX: need to handle conversion failures
                    }
                    default:
                        break;
                }
                return nullptr;
            }
            case syntax::ast_node_types_t::boolean_literal: {
                return make_bool(current_scope(), node->token.as_bool());
            }
            case syntax::ast_node_types_t::else_expression: {
                return evaluate(r, session, node->children[0]);
            }
            case syntax::ast_node_types_t::if_expression:
            case syntax::ast_node_types_t::elseif_expression: {
                auto predicate = evaluate(r, session, node->lhs);
                auto true_branch = evaluate(r, session, node->children[0]);
                auto false_branch = evaluate(r, session, node->rhs);
                return make_if(current_scope(), predicate, true_branch, false_branch);
            }
            case syntax::ast_node_types_t::unary_operator: {
                auto it = s_unary_operators.find(node->token.type);
                if (it == s_unary_operators.end())
                    return nullptr;
                return make_unary_operator(
                    current_scope(),
                    it->second,
                    resolve_symbol_or_evaluate(r, session, node->rhs));
            }
            case syntax::ast_node_types_t::binary_operator: {
                auto it = s_binary_operators.find(node->token.type);
                if (it == s_binary_operators.end())
                    return nullptr;
                auto lhs = resolve_symbol_or_evaluate(r, session, node->lhs);
                auto rhs = resolve_symbol_or_evaluate(r, session, node->rhs);
                return make_binary_operator(current_scope(), it->second, lhs, rhs);
            }
            case syntax::ast_node_types_t::proc_call: {
                qualified_symbol_t qualified_symbol {};
                make_qualified_symbol(qualified_symbol, node->lhs);
                auto proc_identifier = find_identifier(qualified_symbol);

                compiler::argument_list* args = nullptr;
                auto expr = evaluate(r, session, node->rhs);
                if (expr != nullptr) {
                    args = dynamic_cast<compiler::argument_list*>(expr);
                }
                return make_procedure_call(
                    current_scope(),
                    make_identifier_reference(
                        current_scope(),
                        qualified_symbol,
                        proc_identifier),
                    args);
            }
            case syntax::ast_node_types_t::argument_list: {
                auto args = make_argument_list(current_scope());
                for (const auto& arg_node : node->children) {
                    auto arg = resolve_symbol_or_evaluate(r, session, arg_node);
                    args->add(arg);
                    arg->parent_element(args);
                }
                return args;
            }
            case syntax::ast_node_types_t::proc_expression: {
                auto active_scope = current_scope();
                auto block_scope = make_block(
                    active_scope,
                    element_type_t::proc_type_block);
                auto proc_type = make_procedure_type(active_scope, block_scope);
                active_scope->types().add(proc_type);

                auto count = 0;
                for (const auto& type_node : node->lhs->children) {
                    switch (type_node->type) {
                        case syntax::ast_node_types_t::symbol: {
                            auto return_identifier = make_identifier(
                                block_scope,
                                make_symbol(block_scope, fmt::format("_{}", count++)),
                                nullptr);
                            return_identifier->usage(identifier_usage_t::stack);
                            return_identifier->type(find_type(qualified_symbol_t {
                                .name = type_node->children[0]->token.value
                            }));
                            proc_type->returns().add(make_field(block_scope, return_identifier));
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                }

                for (const auto& param_node : node->rhs->children) {
                    switch (param_node->type) {
                        case syntax::ast_node_types_t::assignment: {
                            // XXX: in the parameter list, multiple targets is an error
                            const auto& first_target = param_node->lhs->children[0];
                            auto symbol = dynamic_cast<compiler::symbol_element*>(evaluate_in_scope(
                                r,
                                session,
                                first_target,
                                block_scope));
                            type_find_result_t find_type_result {};
                            find_identifier_type(r, find_type_result, first_target->rhs, block_scope);
                            auto param_identifier = add_identifier_to_scope(
                                r,
                                session,
                                symbol,
                                find_type_result,
                                param_node,
                                block_scope);
                            param_identifier->usage(identifier_usage_t::stack);
                            auto field = make_field(block_scope, param_identifier);
                            proc_type->parameters().add(field);
                            break;
                        }
                        case syntax::ast_node_types_t::symbol: {
                            auto symbol = dynamic_cast<compiler::symbol_element*>(evaluate_in_scope(
                                r,
                                session,
                                param_node,
                                block_scope));
                            type_find_result_t find_type_result {};
                            find_identifier_type(r, find_type_result, param_node->rhs, block_scope);
                            auto param_identifier = add_identifier_to_scope(
                                r,
                                session,
                                symbol,
                                find_type_result,
                                nullptr,
                                block_scope);
                            param_identifier->usage(identifier_usage_t::stack);
                            auto field = make_field(block_scope, param_identifier);
                            proc_type->parameters().add(field);
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                }

                return proc_type;
            }
            case syntax::ast_node_types_t::enum_expression: {
                auto active_scope = current_scope();
                auto enum_type = make_enum_type(
                    r,
                    active_scope,
                    make_block(active_scope, element_type_t::block));
                active_scope->types().add(enum_type);
                add_composite_type_fields(r, session, enum_type, node->rhs);
                if (!enum_type->initialize(r, this))
                    return nullptr;
                return enum_type;
            }
            case syntax::ast_node_types_t::cast_expression: {
                auto type_name = node->lhs->lhs->children[0]->token.value;
                auto type = find_type(qualified_symbol_t {.name = type_name});
                if (type == nullptr) {
                    r.add_message(
                        "P002",
                        fmt::format("unknown type '{}'.", type_name),
                        true);
                }
                return make_cast(
                    current_scope(),
                    type,
                    resolve_symbol_or_evaluate(r, session, node->rhs));
            }
            case syntax::ast_node_types_t::alias_expression: {
                return make_alias(
                    current_scope(),
                    resolve_symbol_or_evaluate(r, session, node->lhs));
            }
            case syntax::ast_node_types_t::union_expression: {
                auto active_scope = current_scope();
                auto union_type = make_union_type(
                    r,
                    active_scope,
                    make_block(active_scope, element_type_t::block));
                active_scope->types().add(union_type);
                add_composite_type_fields(r, session, union_type, node->rhs);
                if (!union_type->initialize(r, this))
                    return nullptr;
                return union_type;
            }
            case syntax::ast_node_types_t::struct_expression: {
                auto active_scope = current_scope();
                auto struct_type = make_struct_type(
                    r,
                    active_scope,
                    make_block(active_scope, element_type_t::block));
                active_scope->types().add(struct_type);
                add_composite_type_fields(r, session, struct_type, node->rhs);
                if (!struct_type->initialize(r, this))
                    return nullptr;
                return struct_type;
            }
            case syntax::ast_node_types_t::return_statement: {
                auto return_element = make_return(current_scope());
                auto& expressions = return_element->expressions();
                for (const auto& arg_node : node->rhs->children) {
                    auto arg = resolve_symbol_or_evaluate(r, session, arg_node);
                    expressions.push_back(arg);
                    arg->parent_element(return_element);
                }
                return return_element;
            }
            case syntax::ast_node_types_t::import_expression: {
                return make_import(
                    current_scope(),
                    resolve_symbol_or_evaluate(r, session, node->lhs));
            }
            case syntax::ast_node_types_t::namespace_expression: {
                return make_namespace(
                    current_scope(),
                    evaluate(r, session, node->rhs, default_block_type));
            }
            default: {
                break;
            }
        }

        return nullptr;
    }

    bool program::on_emit(
            common::result& r,
            emit_context_t& context) {
        auto instruction_block = context.assembler->make_basic_block();
        instruction_block->jump_direct("_initializer");

        std::map<vm::section_t, element_list_t> vars_by_section {};
        auto bss  = vars_by_section.insert(std::make_pair(vm::section_t::bss,     element_list_t()));
        auto ro   = vars_by_section.insert(std::make_pair(vm::section_t::ro_data, element_list_t()));
        auto data = vars_by_section.insert(std::make_pair(vm::section_t::data,    element_list_t()));
        auto text = vars_by_section.insert(std::make_pair(vm::section_t::text,    element_list_t()));

        auto identifiers = elements().find_by_type(element_type_t::identifier);
        for (auto identifier : identifiers) {
            auto var = dynamic_cast<compiler::identifier*>(identifier);
            auto var_type = var->type();
            if (var_type == nullptr
            ||  var_type->element_type() == element_type_t::namespace_type)
                continue;

            if (within_procedure_scope(var->parent_scope())
            ||  var->is_parent_element(element_type_t::field))
                continue;

            switch (var->type()->element_type()) {
                case element_type_t::numeric_type: {
                    if (var->is_constant()) {
                        auto& list = ro.first->second;
                        list.emplace_back(var);
                    }
                    else {
                        auto& list = data.first->second;
                        list.emplace_back(var);
                    }
                    break;
                }
                case element_type_t::any_type:
                case element_type_t::array_type:
                case element_type_t::tuple_type:
                case element_type_t::string_type:
                case element_type_t::composite_type: {
                    if (var->initializer() != nullptr) {
                        if (var->is_constant()) {
                            auto& list = ro.first->second;
                            list.emplace_back(var);
                        } else {
                            auto& list = data.first->second;
                            list.emplace_back(var);
                        }
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }

        auto& ro_list = ro.first->second;
        for (const auto& it : _interned_string_literals) {
            compiler::string_literal* str = it.second.front();
            if (!str->is_parent_element(element_type_t::argument_list))
                continue;
            ro_list.emplace_back(str);
        }

        for (const auto& section : vars_by_section) {
            instruction_block->section(section.first);
            instruction_block->current_entry()->blank_lines(1);

            for (auto e : section.second) {
                switch (e->element_type()) {
                    case element_type_t::string_literal: {
                        auto string_literal = dynamic_cast<compiler::string_literal*>(e);
                        instruction_block->memo();
                        auto it = _interned_string_literals.find(string_literal->value());
                        if (it != _interned_string_literals.end()) {
                            auto current_entry = instruction_block->current_entry();
                            string_literal_list_t& str_list = it->second;
                            for (auto str : str_list) {
                                auto var_label = instruction_block->make_label(str->label_name());
                                current_entry->label(var_label);
                            }
                            current_entry->blank_lines(1);
                        }
                        instruction_block->current_entry()->comment(fmt::format(
                            "\"{}\"",
                            string_literal->value()));
                        instruction_block->string(string_literal->value());
                        break;
                    }
                    case element_type_t::identifier: {
                        auto var = dynamic_cast<compiler::identifier*>(e);
                        auto init = var->initializer();

                        instruction_block->memo();
                        auto var_label = instruction_block->make_label(var->symbol()->name());
                        auto current_entry = instruction_block->current_entry();
                        current_entry->label(var_label);
                        current_entry->blank_lines(1);

                        switch (var->type()->element_type()) {
                            case element_type_t::numeric_type: {
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
                                    auto string_literal = dynamic_cast<compiler::string_literal*>(
                                        init->expression());
                                    instruction_block->current_entry()->comment(fmt::format(
                                        "\"{}\"",
                                        string_literal->value()));
                                    instruction_block->string(string_literal->value());
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

        context.assembler->push_block(instruction_block);

        auto procedure_types = elements().find_by_type(element_type_t::proc_type);
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
            procedure_type->emit(r, context);
        }

        auto top_level_block = context.assembler->make_basic_block();
        top_level_block->align(vm::instruction_t::alignment);
        top_level_block->current_entry()->blank_lines(1);
        top_level_block->memo();
        top_level_block->current_entry()->label(top_level_block->make_label("_initializer"));

        auto all_blocks = elements().find_by_type(element_type_t::block);
        block_list_t implicit_blocks {};
        for (auto block : all_blocks) {
            if (block->is_parent_element(element_type_t::namespace_e)
            ||  block->is_parent_element(element_type_t::program)) {
                implicit_blocks.emplace_back(dynamic_cast<compiler::block*>(block));
            }
        }

        context.assembler->push_block(top_level_block);
        context.push_block(false);
        for (auto block : implicit_blocks)
            block->emit(r, context);
        context.pop();

        auto finalizer_block = context.assembler->make_basic_block();
        finalizer_block->align(vm::instruction_t::alignment);
        finalizer_block->current_entry()->blank_lines(1);
        finalizer_block->exit();
        finalizer_block->current_entry()->label(finalizer_block->make_label("_finalizer"));

        context.assembler->pop_block();
        context.assembler->pop_block();

        return true;
    }

    bool program::compile(
            common::result& r,
            compiler::session& session) {
        _block = push_new_block();
        _block->parent_element(this);

        _top_level_stack.push(_block);

        initialize_core_types(r);

        for (const auto& source_file : session.source_files()) {
            if (!compile_module(r, session, source_file))
                return false;
        }

        auto directives = elements().find_by_type(element_type_t::directive);
        for (auto directive : directives) {
            auto directive_element = dynamic_cast<compiler::directive*>(directive);
            if (!directive_element->execute(r, session, this))
                return false;
        }

        if (!resolve_unknown_types(r))
            return false;

        if (!resolve_unknown_identifiers(r))
            return false;

        if (!r.is_failed()) {
            // XXX: this works for now; eventually need to fix this
            //      so instructions align with original source files
            auto& listing = _assembler->listing();
            listing.add_source_file("top_level.basm");
            listing.select_source_file("top_level.basm");

            emit_context_t context(_terp, _assembler, this);
            emit(r, context);

            // XXX: encode to terp

            // XXX: execute #run directives
        }

        _top_level_stack.pop();

        return !r.is_failed();
    }

    vm::terp* program::terp() {
        return _terp;
    }

    compiler::block* program::block() {
        return _block;
    }

    bool program::compile_module(
            common::result& r,
            compiler::session& session,
            const std::filesystem::path& source_file) {
        session.raise_phase(session_compile_phase_t::start, source_file);

        auto module_node = session.parse(r, source_file);
        if (module_node != nullptr) {
            auto module = dynamic_cast<compiler::module*>(evaluate(
                r,
                session,
                module_node));
            module->parent_element(this);
            module->source_file(source_file);
        }

        if (r.is_failed()) {
            session.raise_phase(session_compile_phase_t::failed, source_file);
            return false;
        } else {
            session.raise_phase(session_compile_phase_t::success, source_file);
            return true;
        }
    }

    element_map& program::elements() {
        return _elements;
    }

    bool program::run(common::result& r) {
        while (!_terp->has_exited())
            if (!_terp->step(r))
                return false;
        return true;
    }

    element* program::evaluate_in_scope(
            common::result& r,
            compiler::session& session,
            const syntax::ast_node_shared_ptr& node,
            compiler::block* scope,
            element_type_t default_block_type) {
        push_scope(scope);
        auto result = evaluate(r, session, node, default_block_type);
        pop_scope();
        return result;
    }

    compiler::block* program::pop_scope() {
        if (_scope_stack.empty())
            return nullptr;
        auto top = _scope_stack.top();
        _scope_stack.pop();
        return top;
    }

    alias* program::make_alias(
            compiler::block* parent_scope,
            element* expr) {
        auto alias_type = new compiler::alias(parent_scope, expr);
        if (expr != nullptr)
            expr->parent_element(alias_type);
        _elements.add(alias_type);
        return alias_type;
    }

    any_type* program::make_any_type(
            common::result& r,
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto type = new compiler::any_type(parent_scope, scope);
        if (!type->initialize(r, this))
            return nullptr;

        _elements.add(type);
        return type;
    }

    type_info* program::make_type_info_type(
            common::result& r,
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto type = new compiler::type_info(parent_scope, scope);
        if (!type->initialize(r, this))
            return nullptr;

        _elements.add(type);
        return type;
    }

    compiler::block* program::current_scope() const {
        if (_scope_stack.empty())
            return nullptr;
        return _scope_stack.top();
    }

    void program::initialize_core_types(common::result& r) {
        auto parent_scope = current_scope();

        compiler::numeric_type::make_types(r, parent_scope, this);
        add_type_to_scope(make_namespace_type(r, parent_scope));
        add_type_to_scope(make_string_type(
            r,
            parent_scope,
            make_block(parent_scope, element_type_t::block)));

        add_type_to_scope(make_type_info_type(
            r,
            parent_scope,
            make_block(parent_scope, element_type_t::block)));
        add_type_to_scope(make_tuple_type(
            r,
            parent_scope,
            make_block(parent_scope, element_type_t::block)));
        add_type_to_scope(make_any_type(
            r,
            parent_scope,
            make_block(parent_scope, element_type_t::block)));
    }

    if_element* program::make_if(
            compiler::block* parent_scope,
            element* predicate,
            element* true_branch,
            element* false_branch) {
        auto if_element = new compiler::if_element(
            parent_scope,
            predicate,
            true_branch,
            false_branch);
        if (predicate != nullptr)
            predicate->parent_element(if_element);
        if (true_branch != nullptr)
            true_branch->parent_element(if_element);
        if (false_branch != nullptr)
            false_branch->parent_element(if_element);
        _elements.add(if_element);
        return if_element;
    }

    comment* program::make_comment(
            compiler::block* parent_scope,
            comment_type_t type,
            const std::string& value) {
        auto comment = new compiler::comment(parent_scope, type, value);
        _elements.add(comment);
        return comment;
    }

    compiler::directive* program::make_directive(
            compiler::block* parent_scope,
            const std::string& name,
            element* expr) {
        auto directive = new compiler::directive(parent_scope, name, expr);
        if (expr != nullptr)
            expr->parent_element(directive);
        _elements.add(directive);
        return directive;
    }

    attribute* program::make_attribute(
            compiler::block* parent_scope,
            const std::string& name,
            element* expr) {
        auto attr = new compiler::attribute(parent_scope, name, expr);
        if (expr != nullptr)
            expr->parent_element(attr);
        _elements.add(attr);
        return attr;
    }

    compiler::symbol_element* program::make_symbol(
            compiler::block* parent_scope,
            const std::string& name,
            const string_list_t& namespaces) {
        auto symbol = new compiler::symbol_element(
            parent_scope,
            name,
            namespaces);
        _elements.add(symbol);
        return symbol;
    }

    compiler::symbol_element* program::make_temp_symbol(
            compiler::block* parent_scope,
            const std::string& name,
            const string_list_t& namespaces) {
        return new compiler::symbol_element(
            parent_scope,
            name,
            namespaces);
    }

    identifier_reference* program::make_identifier_reference(
            compiler::block* parent_scope,
            const qualified_symbol_t& symbol,
            compiler::identifier* identifier) {
        auto reference = new compiler::identifier_reference(
            parent_scope,
            symbol,
            identifier);
        _elements.add(reference);
        if (!reference->resolved())
            _unresolved_identifier_references.emplace_back(reference);
        return reference;
    }

    identifier* program::make_identifier(
            compiler::block* parent_scope,
            compiler::symbol_element* symbol,
            initializer* expr) {
        auto identifier = new compiler::identifier(
            parent_scope,
            symbol,
            expr);

        if (expr != nullptr)
            expr->parent_element(identifier);

        symbol->parent_element(identifier);
        _elements.add(identifier);

        return identifier;
    }

    void program::add_procedure_instance(
            common::result& r,
            compiler::session& session,
            compiler::procedure_type* proc_type,
            const syntax::ast_node_shared_ptr& node) {
        if (node->children.empty())
            return;

        for (const auto& child_node : node->children) {
            switch (child_node->type) {
                case syntax::ast_node_types_t::attribute: {
                    auto attribute = make_attribute(
                        proc_type->scope(),
                        child_node->token.value,
                        evaluate(r, session, child_node->lhs));
                    attribute->parent_element(proc_type);
                    proc_type->attributes().add(attribute);
                    break;
                }
                case syntax::ast_node_types_t::basic_block: {
                    auto basic_block = dynamic_cast<compiler::block*>(evaluate_in_scope(
                        r,
                        session,
                        child_node,
                        proc_type->scope(),
                        element_type_t::proc_instance_block));
                    auto instance = make_procedure_instance(
                        proc_type->scope(),
                        proc_type,
                        basic_block);
                    instance->parent_element(proc_type);
                    proc_type->instances().push_back(instance);
                }
                default:
                    break;
            }
        }
    }

    composite_type* program::make_enum_type(
            common::result& r,
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto type_name = fmt::format("__enum_{}__", common::id_pool::instance()->allocate());
        auto symbol = make_symbol(parent_scope, type_name);
        auto type = new compiler::composite_type(
            parent_scope,
            composite_types_t::enum_type,
            scope,
            symbol);
        symbol->parent_element(type);
        scope->parent_element(type);
        _elements.add(type);
        return type;
    }

    initializer* program::make_initializer(
            compiler::block* parent_scope,
            element* expr) {
        auto initializer = new compiler::initializer(
            parent_scope,
            expr);
        if (expr != nullptr)
            expr->parent_element(initializer);
        _elements.add(initializer);
        return initializer;
    }

    return_element* program::make_return(compiler::block* parent_scope) {
        auto return_element = new compiler::return_element(parent_scope);
        _elements.add(return_element);
        return return_element;
    }

    numeric_type* program::make_numeric_type(
            common::result& r,
            compiler::block* parent_scope,
            const std::string& name,
            int64_t min,
            uint64_t max) {
        auto type = new compiler::numeric_type(
            parent_scope,
            make_symbol(parent_scope, name),
            min,
            max);
        if (!type->initialize(r, this))
            return nullptr;

        _elements.add(type);
        return type;
    }

    compiler::block* program::push_new_block(element_type_t type) {
        auto parent_scope = current_scope();
        auto scope_block = make_block(parent_scope, type);

        if (parent_scope != nullptr)
            parent_scope->blocks().push_back(scope_block);

        push_scope(scope_block);
        return scope_block;
    }

    tuple_type* program::make_tuple_type(
            common::result& r,
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto type = new compiler::tuple_type(parent_scope, scope);
        if (!type->initialize(r, this))
            return nullptr;
        scope->parent_element(type);
        _elements.add(type);
        return type;
    }

    string_type* program::make_string_type(
            common::result& r,
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto type = new compiler::string_type(parent_scope, scope);
        if (!type->initialize(r, this))
            return nullptr;
        scope->parent_element(type);
        _elements.add(type);
        return type;
    }

    namespace_element* program::make_namespace(
            compiler::block* parent_scope,
            element* expr) {
        auto ns = new compiler::namespace_element(
            parent_scope,
            expr);
        if (expr != nullptr)
            expr->parent_element(ns);
        _elements.add(ns);
        return ns;
    }

    composite_type* program::make_union_type(
            common::result& r,
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto type_name = fmt::format("__union_{}__", common::id_pool::instance()->allocate());
        auto symbol = make_symbol(parent_scope, type_name);
        auto type = new compiler::composite_type(
            parent_scope,
            composite_types_t::union_type,
            scope,
            symbol);
        symbol->parent_element(type);
        scope->parent_element(type);
        _elements.add(type);
        return type;
    }

    composite_type* program::make_struct_type(
            common::result& r,
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto type_name = fmt::format("__struct_{}__", common::id_pool::instance()->allocate());
        auto symbol = make_symbol(parent_scope, type_name);
        auto type = new compiler::composite_type(
            parent_scope,
            composite_types_t::struct_type,
            scope,
            symbol);
        symbol->parent_element(type);
        scope->parent_element(type);
        _elements.add(type);
        return type;
    }

    procedure_call* program::make_procedure_call(
            compiler::block* parent_scope,
            compiler::identifier_reference* reference,
            compiler::argument_list* args) {
        auto proc_call = new compiler::procedure_call(
            parent_scope,
            reference,
            args);
        _elements.add(proc_call);
        args->parent_element(proc_call);
        reference->parent_element(proc_call);
        return proc_call;
    }

    argument_list* program::make_argument_list(compiler::block* parent_scope) {
        auto list = new compiler::argument_list(parent_scope);
        _elements.add(list);
        return list;
    }

    unary_operator* program::make_unary_operator(
            compiler::block* parent_scope,
            operator_type_t type,
            element* rhs) {
        auto unary_operator = new compiler::unary_operator(
            parent_scope,
            type,
            rhs);
        rhs->parent_element(unary_operator);
        _elements.add(unary_operator);
        return unary_operator;
    }

    binary_operator* program::make_binary_operator(
            compiler::block* parent_scope,
            operator_type_t type,
            element* lhs,
            element* rhs) {
        auto binary_operator = new compiler::binary_operator(
            parent_scope,
            type,
            lhs,
            rhs);
        lhs->parent_element(binary_operator);
        rhs->parent_element(binary_operator);
        _elements.add(binary_operator);
        return binary_operator;
    }

    label* program::make_label(
            compiler::block* parent_scope,
            const std::string& name) {
        auto label = new compiler::label(parent_scope, name);
        _elements.add(label);
        return label;
    }

    field* program::make_field(
            compiler::block* parent_scope,
            compiler::identifier* identifier) {
        auto field = new compiler::field(parent_scope, identifier);
        identifier->parent_element(field);
        _elements.add(field);
        return field;
    }

    float_literal* program::make_float(
            compiler::block* parent_scope,
            double value) {
        auto literal = new compiler::float_literal(parent_scope, value);
        _elements.add(literal);
        return literal;
    }

    boolean_literal* program::make_bool(
            compiler::block* parent_scope,
            bool value) {
        auto boolean_literal = new compiler::boolean_literal(parent_scope, value);
        _elements.add(boolean_literal);
        return boolean_literal;
    }

    expression* program::make_expression(
            compiler::block* parent_scope,
            element* expr) {
        auto expression = new compiler::expression(parent_scope, expr);
        if (expr != nullptr)
            expr->parent_element(expression);
        _elements.add(expression);
        return expression;
    }

    integer_literal* program::make_integer(
            compiler::block* parent_scope,
            uint64_t value) {
        auto literal = new compiler::integer_literal(parent_scope, value);
        _elements.add(literal);
        return literal;
    }

    void program::push_scope(compiler::block* block) {
        _scope_stack.push(block);
    }

    procedure_instance* program::make_procedure_instance(
            compiler::block* parent_scope,
            compiler::type* procedure_type,
            compiler::block* scope) {
        auto instance = new compiler::procedure_instance(
            parent_scope,
            procedure_type,
            scope);
        scope->parent_element(instance);
        _elements.add(instance);
        return instance;
    }

    compiler::element* program::resolve_symbol_or_evaluate(
            common::result& r,
            compiler::session& session,
            const syntax::ast_node_shared_ptr& node) {
        compiler::element* element = nullptr;
        if (node->type == syntax::ast_node_types_t::symbol) {
            qualified_symbol_t qualified_symbol {};
            make_qualified_symbol(qualified_symbol, node);
            element = make_identifier_reference(
                current_scope(),
                qualified_symbol,
                find_identifier(qualified_symbol));
        } else {
            element = evaluate(r, session, node);
        }
        return element;
    }

    compiler::identifier* program::add_identifier_to_scope(
            common::result& r,
            compiler::session& session,
            compiler::symbol_element* symbol,
            type_find_result_t& type_find_result,
            const syntax::ast_node_shared_ptr& node,
            compiler::block* parent_scope) {
        auto namespace_type = find_type(qualified_symbol_t {
            .name = "namespace"
        });

        auto scope = symbol->is_qualified()
            ? _top_level_stack.top()
            : parent_scope != nullptr ? parent_scope : current_scope();

        auto namespaces = symbol->namespaces();
        string_list_t temp_list {};
        std::string namespace_name {};
        for (size_t i = 0; i < namespaces.size(); i++) {
            if (!namespace_name.empty())
                temp_list.push_back(namespace_name);
            namespace_name = namespaces[i];
            auto var = scope->identifiers().find(namespace_name);
            if (var == nullptr) {
                auto new_scope = make_block(scope, element_type_t::block);
                auto ns = make_namespace(scope, new_scope);
                auto ns_identifier = make_identifier(
                    scope,
                    make_symbol(scope, namespace_name, temp_list),
                    make_initializer(scope, ns));
                ns_identifier->type(namespace_type);
                ns_identifier->inferred_type(true);
                ns_identifier->parent_element(scope->parent_element());
                scope->blocks().push_back(new_scope);
                scope->identifiers().add(ns_identifier);
                scope = new_scope;
            } else {
                auto expr = var->initializer()->expression();
                if (expr->element_type() == element_type_t::namespace_e) {
                    auto ns = dynamic_cast<namespace_element*>(expr);
                    scope = dynamic_cast<compiler::block*>(ns->expression());
                } else {
                    r.add_message(
                        "P018",
                        "only a namespace is valid within a qualified name.",
                        true);
                    return nullptr;
                }
            }
        }

        auto init_expr = (compiler::element*) nullptr;
        auto init = (compiler::initializer*) nullptr;
        if (node != nullptr && node->rhs != nullptr) {
            init_expr = evaluate_in_scope(r, session, node->rhs, scope);
            if (init_expr != nullptr) {
                if (init_expr->is_constant()) {
                    init = make_initializer(scope, init_expr);
                }
            }
        }

        auto new_identifier = make_identifier(scope, symbol, init);
        apply_attributes(r, session, new_identifier, node);
        if (init_expr != nullptr) {
            if (init == nullptr)
                init_expr->parent_element(new_identifier);
            else {
                auto folded_expr = init_expr->fold(r, this);
                if (folded_expr != nullptr) {
                    init_expr = folded_expr;
                    auto old_expr = init->expression();
                    init->expression(init_expr);
                    _elements.remove(old_expr->id());
                }
            }
        }

        if (type_find_result.type == nullptr) {
            if (init_expr != nullptr) {
                type_find_result.type = init_expr->infer_type(this);
                new_identifier->type(type_find_result.type);
                new_identifier->inferred_type(type_find_result.type != nullptr);
            }

            if (type_find_result.type == nullptr) {
                new_identifier->type(make_unknown_type_from_find_result(
                    r,
                    scope,
                    new_identifier,
                    type_find_result));
            }
        } else {
            new_identifier->type(type_find_result.type);
        }

        scope->identifiers().add(new_identifier);

        if (init != nullptr
        &&  init->expression()->element_type() == element_type_t::proc_type) {
            add_procedure_instance(
                r,
                session,
                dynamic_cast<procedure_type*>(init->expression()),
                node->rhs);
        }

        if (init == nullptr && init_expr != nullptr) {
            if (new_identifier->type()->element_type() == element_type_t::unknown_type) {
                r.add_message(
                    "P019",
                    fmt::format("unable to infer type: {}", new_identifier->symbol()->name()),
                    true);
                return nullptr;
            } else {
                auto assign_bin_op = make_binary_operator(
                    scope,
                    operator_type_t::assignment,
                    new_identifier,
                    init_expr);
                add_expression_to_scope(
                    scope,
                    make_statement(current_scope(), label_list_t {}, assign_bin_op));
            }
        }

        return new_identifier;
    }

    cast* program::make_cast(
            compiler::block* parent_scope,
            compiler::type* type,
            element* expr) {
        auto cast = new compiler::cast(parent_scope, type, expr);
        _elements.add(cast);
        return cast;
    }

    bool program::visit_blocks(
            common::result& r,
            const block_visitor_callable& callable,
            compiler::block* root_block) {
        std::function<bool (compiler::block*)> recursive_execute =
            [&](compiler::block* scope) -> bool {
                if (!callable(scope))
                    return false;
                for (auto block : scope->blocks()) {
                    if (!recursive_execute(block))
                        return false;
                }
                return true;
            };
        return recursive_execute(root_block != nullptr ? root_block : _top_level_stack.top());
    }

    void program::apply_attributes(
            common::result& r,
            compiler::session& session,
            compiler::element* element,
            const syntax::ast_node_shared_ptr& node) {
        if (node == nullptr)
            return;

        for (auto it = node->children.begin();
             it != node->children.end();
             ++it) {
            const auto& child_node = *it;
            if (child_node->type == syntax::ast_node_types_t::attribute) {
                auto attribute = dynamic_cast<compiler::attribute*>(evaluate(
                    r,
                    session,
                    child_node));
                attribute->parent_element(element);

                auto& attributes = element->attributes();
                attributes.add(attribute);
            }
        }
    }

    statement* program::make_statement(
            compiler::block* parent_scope,
            label_list_t labels,
            element* expr) {
        auto statement = new compiler::statement(
            parent_scope,
            expr);
        // XXX: double check this
        if (expr != nullptr && expr->parent_element() == nullptr)
            expr->parent_element(statement);
        for (auto label : labels) {
            statement->labels().push_back(label);
            label->parent_element(statement);
        }
        _elements.add(statement);
        return statement;
    }

    string_literal* program::make_string(
            compiler::block* parent_scope,
            const std::string& value) {
        auto literal = new compiler::string_literal(parent_scope, value);
        _elements.add(literal);

        auto it = _interned_string_literals.find(value);
        if (it != _interned_string_literals.end()) {
            auto& list = it->second;
            list.emplace_back(literal);
        } else {
            string_literal_list_t list {};
            list.emplace_back(literal);
            _interned_string_literals.insert(std::make_pair(value, list));
        }

        return literal;
    }

    array_type* program::make_array_type(
            common::result& r,
            compiler::block* parent_scope,
            compiler::block* scope,
            compiler::type* entry_type,
            size_t size) {
        auto type_name = fmt::format("__array_{}_{}__", entry_type->symbol()->name(), size);
        auto symbol = make_symbol(parent_scope, type_name);
        auto type = new compiler::array_type(
            parent_scope,
            symbol,
            scope,
            entry_type);
        if (!type->initialize(r, this))
            return nullptr;
        type->size(size);
        symbol->parent_element(type);
        scope->parent_element(type);
        _elements.add(type);
        return type;
    }

    module* program::make_module(
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto module_element = new compiler::module(parent_scope, scope);
        _elements.add(module_element);
        scope->parent_element(module_element);
        return module_element;
    }

    compiler::block* program::make_block(
            compiler::block* parent_scope,
            element_type_t type) {
        auto block_element = new compiler::block(parent_scope, type);
        _elements.add(block_element);
        return block_element;
    }

    void program::add_composite_type_fields(
            common::result& r,
            compiler::session& session,
            compiler::composite_type* type,
            const syntax::ast_node_shared_ptr& block) {
        auto u32_type = find_type(qualified_symbol_t {.name = "u32"});

        for (const auto& child : block->children) {
            if (child->type != syntax::ast_node_types_t::statement) {
                // XXX: this is an error!
                break;
            }
            auto expr_node = child->rhs;
            switch (expr_node->type) {
                case syntax::ast_node_types_t::assignment: {
                    auto symbol = dynamic_cast<compiler::symbol_element*>(evaluate_in_scope(
                        r,
                        session,
                        expr_node->lhs,
                        type->scope()));
                    type_find_result_t type_find_result {};
                    find_identifier_type(r, type_find_result, expr_node->rhs, type->scope());
                    auto init = make_initializer(
                        type->scope(),
                        evaluate_in_scope(r, session, expr_node->rhs, type->scope()));
                    auto field_identifier = make_identifier(
                        type->scope(),
                        symbol,
                        init);
                    if (type_find_result.type == nullptr) {
                        type_find_result.type = init->expression()->infer_type(this);
                        field_identifier->inferred_type(type_find_result.type != nullptr);
                    }
                    field_identifier->type(type_find_result.type);
                    auto new_field = make_field(
                        type->scope(),
                        field_identifier);
                    new_field->parent_element(type);
                    type->fields().add(new_field);
                    break;
                }
                case syntax::ast_node_types_t::symbol: {
                    auto symbol = dynamic_cast<compiler::symbol_element*>(evaluate_in_scope(
                        r,
                        session,
                        expr_node,
                        type->scope()));
                    type_find_result_t type_find_result {};
                    find_identifier_type(r, type_find_result, expr_node->rhs, type->scope());
                    auto field_identifier = make_identifier(
                        type->scope(),
                        symbol,
                        nullptr);
                    if (type_find_result.type == nullptr) {
                        if (type->type() == composite_types_t::enum_type) {
                            field_identifier->type(u32_type);
                        } else {
                            field_identifier->type(make_unknown_type_from_find_result(
                                r,
                                type->scope(),
                                field_identifier,
                                type_find_result));
                        }
                    } else {
                        field_identifier->type(type_find_result.type);
                    }
                    auto new_field = make_field(
                        type->scope(),
                        field_identifier);
                    new_field->parent_element(type);
                    type->fields().add(new_field);
                    break;
                }
                default:
                    break;
            }
        }
    }

    unknown_type* program::make_unknown_type(
            common::result& r,
            compiler::block* parent_scope,
            compiler::symbol_element* symbol,
            bool is_array,
            size_t array_size) {
        auto type = new compiler::unknown_type(parent_scope, symbol);
        if (!type->initialize(r, this))
            return nullptr;
        type->is_array(is_array);
        type->array_size(array_size);
        symbol->parent_element(type);
        _elements.add(type);
        return type;
    }

    procedure_type* program::make_procedure_type(
            compiler::block* parent_scope,
            compiler::block* block_scope) {
        auto type_name = fmt::format("__proc_{}__", common::id_pool::instance()->allocate());
        auto type = new compiler::procedure_type(
            parent_scope,
            block_scope,
            make_symbol(parent_scope, type_name));
        if (block_scope != nullptr)
            block_scope->parent_element(type);
        _elements.add(type);
        return type;
    }

    void program::add_type_to_scope(compiler::type* type) {
        current_scope()->types().add(type);
    }

    bool program::resolve_unknown_types(common::result& r) {
        auto it = _identifiers_with_unknown_types.begin();
        while (it != _identifiers_with_unknown_types.end()) {
            auto var = *it;

            if (var->type() != nullptr
            &&  var->type()->element_type() != element_type_t::unknown_type) {
                it = _identifiers_with_unknown_types.erase(it);
                continue;
            }

            compiler::type* identifier_type = nullptr;
            if (var->initializer() == nullptr) {
                auto unknown_type = dynamic_cast<compiler::unknown_type*>(var->type());
                identifier_type = find_type({.name = unknown_type->symbol()->name()});
                if (unknown_type->is_array()) {
                    auto array_type = find_array_type(
                        identifier_type,
                        unknown_type->array_size());
                    if (array_type == nullptr) {
                        array_type = make_array_type(
                            r,
                            var->parent_scope(),
                            make_block(var->parent_scope(), element_type_t::block),
                            identifier_type,
                            unknown_type->array_size());
                    }
                    identifier_type = array_type;
                }

                if (identifier_type != nullptr) {
                    var->type(identifier_type);
                    _elements.remove(unknown_type->id());
                }
            } else {
                identifier_type = var
                    ->initializer()
                    ->expression()
                    ->infer_type(this);
                var->type(identifier_type);
            }

            if (identifier_type != nullptr) {
                var->inferred_type(true);
                it = _identifiers_with_unknown_types.erase(it);
            } else {
                ++it;
                r.add_message(
                    "P004",
                    fmt::format("unable to resolve type for identifier: {}", var->symbol()->name()),
                    true);
            }
        }

        return _identifiers_with_unknown_types.empty();
    }

    bool program::resolve_unknown_identifiers(common::result& r) {
        auto it = _unresolved_identifier_references.begin();
        while (it != _unresolved_identifier_references.end()) {
            auto unresolved_reference = *it;
            if (unresolved_reference->resolved()) {
                it = _unresolved_identifier_references.erase(it);
                continue;
            }

            identifier_list_t candidates {};
            auto all_identifiers = _elements.find_by_type(element_type_t::identifier);
            for (auto element : all_identifiers) {
                auto identifier = dynamic_cast<compiler::identifier*>(element);
                if (identifier->symbol()->name() == unresolved_reference->symbol().name)
                    candidates.emplace_back(identifier);
            }

            if (candidates.empty()) {
                ++it;
                r.add_message(
                    "P004",
                    fmt::format(
                        "unable to resolve identifier: {}",
                        unresolved_reference->symbol().name),
                    true);
                continue;
            }

            auto winner = candidates.front();
            unresolved_reference->identifier(winner);

            it = _unresolved_identifier_references.erase(it);
        }

        return _unresolved_identifier_references.empty();
    }

    namespace_type* program::make_namespace_type(
            common::result& r,
            compiler::block* parent_scope) {
        auto type = new compiler::namespace_type(parent_scope);
        if (!type->initialize(r, this))
            return nullptr;

        _elements.add(type);
        return type;
    }

    bool program::find_identifier_type(
            common::result& r,
            type_find_result_t& result,
            const syntax::ast_node_shared_ptr& type_node,
            compiler::block* parent_scope) {
        if (type_node == nullptr)
            return false;

        parent_scope = parent_scope == nullptr ? current_scope() : parent_scope;
        make_qualified_symbol(result.type_name, type_node->lhs);
        result.is_array = type_node->is_array();
        result.array_size = 0; // XXX: this needs to be fixed!
        result.type = find_type(result.type_name);
        if (result.is_array) {
            auto array_type = find_array_type(
                result.type,
                result.array_size);
            if (array_type == nullptr) {
                array_type = make_array_type(
                    r,
                    parent_scope,
                    make_block(parent_scope, element_type_t::block),
                    result.type,
                    result.array_size);
            }
            result.type = array_type;
        }

        return result.type != nullptr;
    }

    unknown_type* program::make_unknown_type_from_find_result(
            common::result& r,
            compiler::block* scope,
            compiler::identifier* identifier,
            const type_find_result_t& result) {
        auto symbol = make_symbol(
            scope,
            result.type_name.name,
            result.type_name.namespaces);
        auto unknown_type = make_unknown_type(
            r,
            scope,
            symbol,
            result.is_array,
            result.array_size);
        _identifiers_with_unknown_types.push_back(identifier);
        return unknown_type;
    }

    void program::make_qualified_symbol(
            qualified_symbol_t& symbol,
            const syntax::ast_node_shared_ptr& node) {
        if (!node->children.empty()) {
            for (size_t i = 0; i < node->children.size() - 1; i++)
                symbol.namespaces.push_back(node->children[i]->token.value);
        }
        symbol.name = node->children.back()->token.value;
    }

    compiler::symbol_element* program::make_symbol_from_node(
            common::result& r,
            const syntax::ast_node_shared_ptr& node) {
        qualified_symbol_t qualified_symbol {};
        make_qualified_symbol(qualified_symbol, node);
        auto symbol = make_symbol(
            current_scope(),
            qualified_symbol.name,
            qualified_symbol.namespaces);
        symbol->constant(node->is_constant_expression());
        return symbol;
    }

    compiler::type* program::find_type(const qualified_symbol_t& symbol) const {
        if (symbol.is_qualified()) {
            auto non_const_this = const_cast<compiler::program*>(this);
            auto block_scope = non_const_this->_top_level_stack.top();
            for (const auto& namespace_name : symbol.namespaces) {
                auto var = block_scope->identifiers().find(namespace_name);
                if (var == nullptr || var->initializer() == nullptr)
                    return nullptr;
                auto expr = var->initializer()->expression();
                if (expr->element_type() == element_type_t::namespace_e) {
                    auto ns = dynamic_cast<namespace_element*>(expr);
                    block_scope = dynamic_cast<compiler::block*>(ns->expression());
                } else {
                    return nullptr;
                }
            }

            auto matching_type = block_scope->types().find(symbol.name);
            if (matching_type != nullptr)
                return matching_type;

            auto type_identifier = block_scope->identifiers().find(symbol.name);
            if (type_identifier != nullptr)
                return type_identifier->type();
        } else {
            auto scope = current_scope();
            while (scope != nullptr) {
                auto type = scope->types().find(symbol.name);
                if (type != nullptr)
                    return type;
                auto type_identifier = scope->identifiers().find(symbol.name);
                if (type_identifier != nullptr)
                    return type_identifier->type();
                scope = scope->parent_scope();
            }
        }
        return nullptr;
    }

    bool program::within_procedure_scope(compiler::block* parent_scope) const {
        auto block_scope = parent_scope == nullptr ? current_scope() : parent_scope;
        while (block_scope != nullptr) {
            if (block_scope->element_type() == element_type_t::proc_type_block
            ||  block_scope->element_type() == element_type_t::proc_instance_block)
                return true;
            block_scope = block_scope->parent_scope();
        }
        return false;
    }

    import* program::make_import(compiler::block* parent_scope, element* expr) {
        auto import_element = new compiler::import(parent_scope, expr);
        _elements.add(import_element);
        return import_element;
    }

    compiler::identifier* program::find_identifier(const qualified_symbol_t& symbol) {
        if (symbol.is_qualified()) {
            auto block_scope = _top_level_stack.top();
            for (const auto& namespace_name : symbol.namespaces) {
                auto var = block_scope->identifiers().find(namespace_name);
                if (var == nullptr || var->initializer() == nullptr)
                    return nullptr;
                auto expr = var->initializer()->expression();
                if (expr->element_type() == element_type_t::namespace_e) {
                    auto ns = dynamic_cast<namespace_element*>(expr);
                    block_scope = dynamic_cast<compiler::block*>(ns->expression());
                } else {
                    return nullptr;
                }
            }
            return block_scope->identifiers().find(symbol.name);
        } else {
            auto block_scope = current_scope();
            while (block_scope != nullptr) {
                auto var = block_scope->identifiers().find(symbol.name);
                if (var != nullptr)
                    return var;
                block_scope = block_scope->parent_scope();
            }
            return nullptr;
        }
    }

    compiler::type* program::find_array_type(compiler::type* entry_type, size_t size) {
        auto type_name = fmt::format(
            "__array_{}_{}__",
            entry_type->symbol()->name(),
            size);
        return find_type(qualified_symbol_t { .name = type_name });
    }

    void program::add_expression_to_scope(compiler::block* scope, compiler::element* expr) {
        switch (expr->element_type()) {
            case element_type_t::comment: {
                auto comment = dynamic_cast<compiler::comment*>(expr);
                scope->comments().push_back(comment);
                break;
            }
            case element_type_t::attribute: {
                auto attribute = dynamic_cast<compiler::attribute*>(expr);
                scope->attributes().add(attribute);
                break;
            }
            case element_type_t::statement: {
                auto statement = dynamic_cast<compiler::statement*>(expr);
                scope->statements().push_back(statement);
                break;
            }
            default:
                break;
        }
    }

};