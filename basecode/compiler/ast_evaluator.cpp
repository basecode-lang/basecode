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

#include <compiler/session.h>
#include "elements.h"
#include "element_map.h"
#include "scope_manager.h"
#include "ast_evaluator.h"
#include "element_builder.h"

namespace basecode::compiler {

    std::unordered_map<syntax::ast_node_type_t, node_evaluator_callable> ast_evaluator::s_node_evaluators = {
        {syntax::ast_node_type_t::pair,                    std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::label,                   std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::module,                  std::bind(&ast_evaluator::module, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::symbol,                  std::bind(&ast_evaluator::symbol, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::type_list,               std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::proc_call,               std::bind(&ast_evaluator::proc_call, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::statement,               std::bind(&ast_evaluator::statement, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::attribute,               std::bind(&ast_evaluator::attribute, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::directive,               std::bind(&ast_evaluator::directive, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::raw_block,               std::bind(&ast_evaluator::raw_block, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::assignment,              std::bind(&ast_evaluator::assignment, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::expression,              std::bind(&ast_evaluator::expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::nil_literal,             std::bind(&ast_evaluator::nil, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::basic_block,             std::bind(&ast_evaluator::basic_block, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::symbol_part,             std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::new_literal,             std::bind(&ast_evaluator::new_literal, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::line_comment,            std::bind(&ast_evaluator::line_comment, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::block_comment,           std::bind(&ast_evaluator::block_comment, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::argument_list,           std::bind(&ast_evaluator::argument_list, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::if_expression,           std::bind(&ast_evaluator::if_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::array_literal,           std::bind(&ast_evaluator::array_literal, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::tuple_literal,           std::bind(&ast_evaluator::tuple_literal, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::parameter_list,          std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::number_literal,          std::bind(&ast_evaluator::number_literal, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::string_literal,          std::bind(&ast_evaluator::string_literal, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::unary_operator,          std::bind(&ast_evaluator::unary_operator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::cast_expression,         std::bind(&ast_evaluator::cast_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::case_expression,         std::bind(&ast_evaluator::case_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::spread_operator,         std::bind(&ast_evaluator::spread_operator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::from_expression,         std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::proc_expression,         std::bind(&ast_evaluator::proc_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::enum_expression,         std::bind(&ast_evaluator::enum_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::binary_operator,         std::bind(&ast_evaluator::binary_operator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::boolean_literal,         std::bind(&ast_evaluator::boolean_literal, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::else_expression,         std::bind(&ast_evaluator::else_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::break_statement,         std::bind(&ast_evaluator::break_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::with_expression,         std::bind(&ast_evaluator::with_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::while_statement,         std::bind(&ast_evaluator::while_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::type_declaration,        std::bind(&ast_evaluator::type_declaration, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::defer_expression,        std::bind(&ast_evaluator::defer_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::union_expression,        std::bind(&ast_evaluator::union_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::return_statement,        std::bind(&ast_evaluator::return_statement, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::symbol_reference,        std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::for_in_statement,        std::bind(&ast_evaluator::for_in_statement, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::family_expression,       std::bind(&ast_evaluator::family_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::switch_expression,       std::bind(&ast_evaluator::switch_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::lambda_expression,       std::bind(&ast_evaluator::lambda_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::switch_expression,       std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::import_expression,       std::bind(&ast_evaluator::import_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::struct_expression,       std::bind(&ast_evaluator::struct_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::character_literal,       std::bind(&ast_evaluator::character_literal, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::module_expression,       std::bind(&ast_evaluator::module_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::elseif_expression,       std::bind(&ast_evaluator::if_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::subscript_operator,      std::bind(&ast_evaluator::subscript_operator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::with_member_access,      std::bind(&ast_evaluator::with_member_access, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::continue_statement,      std::bind(&ast_evaluator::continue_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::type_tagged_symbol,      std::bind(&ast_evaluator::symbol, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::constant_assignment,     std::bind(&ast_evaluator::assignment, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::transmute_expression,    std::bind(&ast_evaluator::transmute_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::namespace_expression,    std::bind(&ast_evaluator::namespace_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::return_argument_list,    std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::array_subscript_list,    std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::uninitialized_literal,   std::bind(&ast_evaluator::uninitialized_literal, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::fallthrough_statement,   std::bind(&ast_evaluator::fallthrough_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::assignment_source_list,  std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_type_t::assignment_target_list,  std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
    };

    ///////////////////////////////////////////////////////////////////////////

    void evaluator_context_t::apply_comments(compiler::element* element) const {
        for (auto comment : comments) {
            comment->parent_element(element);
            element->comments().emplace_back(comment);
        }
    }

    void evaluator_context_t::apply_attributes(compiler::element* element) const {
        for (auto attribute : attributes.as_list()) {
            attribute->parent_element(element);
            element->attributes().add(attribute);
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_evaluator::ast_evaluator(compiler::session& session) : _session(session) {
    }

    element* ast_evaluator::evaluate(const syntax::ast_node_t* node) {
        if (node == nullptr || _session.result().is_failed())
            return nullptr;

        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        evaluator_context_t context;
        context.node = node;
        context.scope = scope_manager.current_scope();

        for (auto attribute : node->attributes) {
            context.attributes.add(builder.make_attribute(
                scope_manager.current_scope(),
                attribute->token.value,
                evaluate(attribute->lhs)));
        }

        for (auto comment : node->comments) {
            switch (comment->type) {
                case syntax::ast_node_type_t::line_comment: {
                    context.comments.emplace_back(builder.make_comment(
                        scope_manager.current_scope(),
                        comment_type_t::line,
                        comment->token.value));
                    break;
                }
                case syntax::ast_node_type_t::block_comment: {
                    context.comments.emplace_back(builder.make_comment(
                        scope_manager.current_scope(),
                        comment_type_t::block,
                        comment->token.value));
                    break;
                }
                default:
                    break;
            }
        }

        auto it = s_node_evaluators.find(node->type);
        if (it != s_node_evaluators.end()) {
            evaluator_result_t result {};
            if (it->second(this, context, result)) {
                context.apply_attributes(result.element);
                context.apply_comments(result.element);

                if (result.element->element_type() == element_type_t::statement) {
                    auto stmt = dynamic_cast<compiler::statement*>(result.element);
                    auto expr = stmt->expression();
                    if (expr != nullptr
                    &&  expr->element_type() == element_type_t::directive) {
                        auto directive_element = dynamic_cast<compiler::directive*>(expr);
                        directive_element->evaluate(_session);
                    }
                }

                return result.element;
            }
        }

        return nullptr;
    }

    bool ast_evaluator::add_procedure_instance(
            const evaluator_context_t& context,
            compiler::procedure_type* proc_type,
            const syntax::ast_node_t* node) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        if (node->children.empty())
            return true;

        for (auto& attr : node->attributes) {
            auto attribute = builder.make_attribute(
                proc_type->scope(),
                attr->token.value,
                evaluate(attr->lhs));
            attribute->parent_element(proc_type);
            proc_type->attributes().add(attribute);
        }

        for (auto child_node : node->children) {
            switch (child_node->type) {
                case syntax::ast_node_type_t::basic_block: {
                    auto basic_block = dynamic_cast<compiler::block*>(evaluate_in_scope(
                        child_node,
                        proc_type->scope()));
                    if (basic_block == nullptr) {
                        _session.error(
                            scope_manager.current_module(),
                            "X000",
                            "unable to evaluate procedure instance block.",
                            child_node->location);
                        return false;
                    }

                    basic_block->has_stack_frame(true);

                    auto return_type_field = proc_type->return_type();
                    if (return_type_field != nullptr) {
                        auto has_return = false;
                        scope_manager.visit_child_blocks(
                            _session.result(),
                            [&](compiler::block* scope) {
                                for (auto stmt : scope->statements()) {
                                    auto return_e = dynamic_cast<compiler::return_element*>(stmt->expression());
                                    if (return_e != nullptr) {
                                        const auto& list = return_e->expressions();
                                        if (!list.empty())
                                            return_e->field(return_type_field);
                                        has_return = true;
                                    }
                                }
                                return true;
                            },
                            basic_block);

                        if (!has_return) {
                            _session.error(
                                scope_manager.current_module(),
                                "X000",
                                "procedure declares return type but has no return statement.",
                                proc_type->location());
                            return false;
                        }

                        proc_type->has_return(true);
                    }

                    auto instance = builder.make_procedure_instance(
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

        return true;
    }

    void ast_evaluator::add_expression_to_scope(
            compiler::block* scope,
            compiler::element* expr) {
        switch (expr->element_type()) {
            case element_type_t::import_e: {
                auto import = dynamic_cast<compiler::import*>(expr);
                scope->imports().emplace_back(import);
                break;
            }
            case element_type_t::attribute: {
                auto attribute = dynamic_cast<compiler::attribute*>(expr);
                scope->attributes().add(attribute);
                break;
            }
            case element_type_t::statement: {
                auto statement = dynamic_cast<compiler::statement*>(expr);
                scope->statements().emplace_back(statement);
                break;
            }
            default:
                break;
        }
    }

    compiler::element* ast_evaluator::evaluate_in_scope(
            const syntax::ast_node_t* node,
            compiler::block* scope) {
        auto& scope_manager = _session.scope_manager();

        if (scope != nullptr)
            scope_manager.push_scope(scope);

        auto result = evaluate(node);

        if (scope != nullptr)
            scope_manager.pop_scope();

        return result;
    }

    compiler::block* ast_evaluator::add_namespaces_to_scope(
            const evaluator_context_t& context,
            const syntax::ast_node_t* node,
            compiler::symbol_element* symbol,
            compiler::block* parent_scope) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        auto namespace_type = scope_manager.find_namespace_type();
        auto namespace_type_ref = builder.make_type_reference(
            parent_scope,
            namespace_type->symbol()->qualified_symbol(),
            namespace_type);

        auto namespaces = symbol->namespaces();
        auto scope = parent_scope;
        string_list_t temp_list {};
        std::string namespace_name {};
        for (const auto& name : namespaces) {
            if (!namespace_name.empty())
                temp_list.push_back(namespace_name);
            namespace_name = name;
            auto vars = scope->identifiers().find(namespace_name);
            if (vars.empty()) {
                auto new_scope = builder.make_block(scope);
                auto ns = builder.make_namespace(scope, new_scope);
                auto ns_identifier = builder.make_identifier(
                    scope,
                    builder.make_symbol(scope, namespace_name, temp_list),
                    builder.make_initializer(scope, ns));
                ns_identifier->type_ref(namespace_type_ref);
                ns_identifier->inferred_type(true);
                ns_identifier->parent_element(scope->parent_element());
                scope->blocks().push_back(new_scope);
                scope->identifiers().add(ns_identifier);
                scope = new_scope;
            } else {
                auto expr = vars.front()->initializer()->expression();
                if (expr->element_type() == element_type_t::namespace_e) {
                    auto ns = dynamic_cast<namespace_element*>(expr);
                    scope = dynamic_cast<compiler::block*>(ns->expression());
                } else {
                    _session.error(
                        scope_manager.current_module(),
                        "P018",
                        "only a namespace is valid within a qualified name.",
                        node->lhs->location);
                    return nullptr;
                }
            }
        }
        return scope;
    }

    void ast_evaluator::add_type_parameters(
            const evaluator_context_t& context,
            compiler::block* scope,
            const syntax::ast_node_t* type_parameters_node,
            type_map_t& type_parameters) {
        if (type_parameters_node->children.empty())
            return;

        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();
        auto open_generic_type = scope_manager.find_generic_type({});

        for (auto type_parameter_node : type_parameters_node->children) {
            compiler::type* generic_type = open_generic_type;
            auto param_symbol = builder.make_symbol_from_node(
                type_parameter_node->rhs,
                scope);
            auto known_type = scope_manager.find_type(param_symbol->qualified_symbol());

            if (known_type == nullptr
            &&  type_parameter_node->lhs != nullptr) {
                compiler::type_reference_list_t constraints {};
                for (auto symbol : type_parameter_node->lhs->rhs->children) {
                    qualified_symbol_t qualified_symbol {};
                    builder.make_qualified_symbol(qualified_symbol, symbol);

                    auto type = scope_manager.find_type(qualified_symbol, scope);
                    auto type_ref = builder.make_type_reference(scope, type->name(), type);
                    constraints.push_back(type_ref);
                }

                auto root_scope = _session.program().block();
                auto constrained_generic_type = scope_manager.find_generic_type(
                    constraints,
                    root_scope);
                if (constrained_generic_type == nullptr) {
                    generic_type = builder.make_generic_type(
                        root_scope,
                        constraints);
                    scope->types().add(generic_type);
                } else {
                    generic_type = constrained_generic_type;
                }
            }

            auto type = known_type != nullptr ? known_type : generic_type;
            auto param_type_ref = builder.make_type_reference(
                scope,
                param_symbol->name(),
                type);
            auto decl = add_identifier_to_scope(
                context,
                param_symbol,
                param_type_ref,
                nullptr,
                0,
                scope);
            decl->identifier()->symbol()->constant(true);
            type_parameters.add(param_symbol, type);
        }
    }

    bool ast_evaluator::add_composite_type_fields(
            evaluator_context_t& context,
            compiler::composite_type* type,
            const syntax::ast_node_t* block) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        compiler::field* previous_field = nullptr;

        uint64_t value = 0;
        auto is_enum = type->is_enum();
        auto is_union = type->is_union();
        std::string value_type_name = "u32";
        compiler::numeric_type* value_type = nullptr;
        if (is_enum) {
            if (type->type_parameters().size() == 1) {
                auto names = type->type_parameters().name_list();
                value_type_name = names[0];
                value_type = dynamic_cast<compiler::numeric_type*>(
                    type->type_parameters().find(value_type_name));
            } else {
                value_type = dynamic_cast<compiler::numeric_type*>(scope_manager
                    .find_type(qualified_symbol_t(value_type_name)));
            }
            if (value_type == nullptr) {
                _session.error(
                    type->module(),
                    "X000",
                    fmt::format("invalid numeric type: {}", value_type_name),
                    type->location());
                return false;
            }

            context.decl_type_ref = builder.make_type_reference(
                type->scope(),
                value_type->symbol()->qualified_symbol(),
                value_type);
        }

        for (auto child : block->children) {
            if (child->type != syntax::ast_node_type_t::statement) {
                break;
            }

            auto expr_node = child->rhs;
            if (expr_node == nullptr)
                continue;

            if (is_enum && value > value_type->max()) {
                _session.error(
                    scope_manager.current_module(),
                    "X000",
                    fmt::format("enum field value exceeds range, type: {}.", value_type_name),
                    expr_node->location);
                return false;
            }

            uint64_t offset = 0;
            if (!is_union && previous_field != nullptr)
                offset = previous_field->end_offset();

            switch (expr_node->type) {
                case syntax::ast_node_type_t::pair: {
                    if (is_enum || is_union) {
                        _session.error(
                            scope_manager.current_module(),
                            "X000",
                            "consolidated field declarations are not supported for enum or union.",
                            expr_node->location);
                        return false;
                    }
                    symbol_list_and_type_t result {};
                    pairs_to_symbols_and_type(expr_node, result);

                    if (result.type_ref == nullptr) {
                        _session.error(
                            scope_manager.current_module(),
                            "X000",
                            "consolidated field declarations require a valid type declaration.",
                            expr_node->location);
                        return false;
                    }

                    for (auto symbol : result.symbols) {
                        auto field_decl = add_identifier_to_scope(
                            context,
                            dynamic_cast<compiler::symbol_element*>(symbol),
                            result.type_ref,
                            nullptr,
                            0,
                            type->scope());
                        if (field_decl != nullptr) {
                            auto new_field = builder.make_field(
                                type,
                                type->scope(),
                                field_decl,
                                offset);
                            type->fields().add(new_field);
                            field_decl->identifier()->field(new_field);
                            previous_field = new_field;
                            offset = previous_field->end_offset();
                        }
                    }
                    break;
                }
                case syntax::ast_node_type_t::assignment:
                case syntax::ast_node_type_t::constant_assignment: {
                    element_list_t list {};
                    auto success = add_assignments_to_scope(
                        context,
                        expr_node,
                        list,
                        type->scope());
                    if (!success) {
                        // XXX: error
                        return false;
                    }
                    for (auto d : list) {
                        auto decl = dynamic_cast<compiler::declaration*>(d);
                        auto newField = builder.make_field(
                            type,
                            type->scope(),
                            decl,
                            offset);
                        type->fields().add(newField);
                        decl->identifier()->field(newField);
                        if (is_enum) {
                            uint64_t init_value;
                            if (!decl->identifier()->as_integer(init_value)) {
                                _session.error(
                                    scope_manager.current_module(),
                                    "X000",
                                    "enum field initializers must be constant integer expressions.",
                                    expr_node->location);
                                return false;
                            }

                            if (init_value < value) {
                                _session.error(
                                    scope_manager.current_module(),
                                    "X000",
                                    "enum field initializers must be equal to or greater than implicit values.",
                                    expr_node->location);
                                return false;
                            }

                            value = ++init_value;
                            decl->identifier()->symbol()->constant(true);
                        }
                        previous_field = newField;
                    }
                    break;
                }
                case syntax::ast_node_type_t::symbol: {
                    auto field_decl = declare_identifier(
                        context,
                        expr_node,
                        type->scope());
                    if (field_decl != nullptr) {
                        if (is_enum) {
                            auto value_expr = builder.make_integer(type->scope(), value++);
                            field_decl->identifier()->initializer(builder.make_initializer(
                                type->scope(),
                                value_expr));
                            field_decl->identifier()->symbol()->constant(true);
                        }
                        auto new_field = builder.make_field(
                            type,
                            type->scope(),
                            field_decl,
                            offset);
                        type->fields().add(new_field);
                        field_decl->identifier()->field(new_field);
                        previous_field = new_field;
                    }
                    break;
                }
                default: {
                    return false;
                }
            }
        }

        return true;
    }

    compiler::element* ast_evaluator::resolve_symbol_or_evaluate(
            const syntax::ast_node_t* node,
            compiler::block* scope,
        bool flag_as_unresolved) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        compiler::element* element = nullptr;
        if (node != nullptr
        &&  node->type == syntax::ast_node_type_t::symbol) {
            qualified_symbol_t qualified_symbol {};
            builder.make_qualified_symbol(qualified_symbol, node);

            auto vars = scope_manager.find_identifier(qualified_symbol, scope);
            compiler::identifier* identifier = vars.empty() ? nullptr : vars.front();

            element = builder.make_identifier_reference(
                scope_manager.current_scope(),
                qualified_symbol,
                identifier,
                flag_as_unresolved);
            element->location(node->location);
        } else {
            if (scope != nullptr)
                element = evaluate_in_scope(node, scope);
            else
                element = evaluate(node);
        }

        return element;
    }

    compiler::declaration* ast_evaluator::add_identifier_to_scope(
            const evaluator_context_t& context,
            compiler::symbol_element* symbol,
            compiler::type_reference* type_ref,
            const syntax::ast_node_t* node,
            size_t source_index,
            compiler::block* parent_scope) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        auto scope = symbol->is_qualified() ?
            scope_manager.current_module()->scope() :
            parent_scope != nullptr ? parent_scope : scope_manager.current_scope();

        scope = add_namespaces_to_scope(context, node, symbol, scope);

        syntax::ast_node_t* source_node = nullptr;
        if (node != nullptr) {
            source_node = node->rhs->children[source_index];
        }

        auto init_expr = (compiler::element*) nullptr;
        auto init = (compiler::initializer*) nullptr;
        if (node != nullptr) {
            init_expr = evaluate_in_scope(source_node, scope);
            if (init_expr != nullptr) {
                if (init_expr->element_type() == element_type_t::symbol) {
                    auto init_symbol = dynamic_cast<compiler::symbol_element*>(init_expr);
                    auto vars = scope_manager.find_identifier(
                        init_symbol->qualified_symbol(),
                        scope);
                    compiler::identifier* identifier = vars.empty() ? nullptr : vars.front();
                    init_expr = builder.make_identifier_reference(
                        scope,
                        init_symbol->qualified_symbol(),
                        identifier);
                }
                if (init_expr->is_constant()) {
                    init = builder.make_initializer(scope, init_expr);

                    // XXX: after a type_literal is created, we could update
                    //      the type_reference's symbol (the first type_params entry).
                    //
                    //      do we want to do this?
                }
            } else {
                if (_session.result().is_failed())
                    return nullptr;
            }
        }

        // XXX: clean up
        if (!symbol->is_constant()) {
            auto is_module = init_expr != nullptr
                && init_expr->element_type() == element_type_t::module_reference;
            if (is_module) {
                _session.error(
                    _session.scope_manager().current_module(),
                    "P029",
                    "constant assignment (::) is required for module references.",
                    node->location);
                return nullptr;
            }

            auto is_ns = init_expr != nullptr
                && init_expr->element_type() == element_type_t::namespace_e;
            if (is_ns) {
                _session.error(
                    _session.scope_manager().current_module(),
                    "P029",
                    "constant assignment (::) is required for namespaces.",
                    node->location);
                return nullptr;
            }

            auto is_type = init_expr != nullptr && init_expr->is_type();
            auto is_type_directive = init_expr != nullptr
                && init_expr->is_directive_of_type(directive_type_t::type);
            if (is_type || is_type_directive) {
                _session.error(
                    _session.scope_manager().current_module(),
                    "P029",
                    "constant assignment (::) is required for types.",
                    node->location);
                return nullptr;
            }
        }

        auto new_identifier = builder.make_identifier(scope, symbol, init);
        if (init_expr != nullptr) {
            // XXX: why?
            if (init == nullptr)
                init_expr->parent_element(new_identifier);
        }

        if (init_expr != nullptr && type_ref == nullptr) {
            infer_type_result_t infer_type_result {};
            if (!init_expr->infer_type(_session, infer_type_result)) {
                // XXX: need to refactor this code to recursively determine if there
                //      are unresolved identifier_reference instances and flag them as
                //      appropriate.
                if (init_expr->element_type() == element_type_t::identifier_reference) {
                    auto ref = dynamic_cast<compiler::identifier_reference*>(init_expr);
                    if (!ref->resolved()) {
                        _session.error(
                            scope_manager.current_module(),
                            "P004",
                            fmt::format("unable to resolve identifier: {}", ref->symbol().name),
                            ref->symbol().location);
                    }
                    return nullptr;
                } else {
                    _session.error(
                        scope_manager.current_module(),
                        "P019",
                        fmt::format("unable to infer type: {}", new_identifier->symbol()->name()),
                        new_identifier->symbol()->location());
                    return nullptr;
                }
            }

            if (infer_type_result.reference == nullptr) {
                infer_type_result.reference = builder.make_type_reference(
                    scope,
                    infer_type_result.inferred_type->symbol()->qualified_symbol(),
                    infer_type_result.inferred_type);
            }

            new_identifier->type_ref(infer_type_result.reference);
            new_identifier->inferred_type(infer_type_result.inferred_type != nullptr);

            if (infer_type_result.reference->is_unknown_type()) {
                _session.scope_manager()
                    .identifiers_with_unknown_types()
                    .push_back(new_identifier);
            }
        } else {
            if (type_ref == nullptr) {
                _session.error(
                    scope_manager.current_module(),
                    "P019",
                    fmt::format("unable to infer type: {}", new_identifier->symbol()->name()),
                    new_identifier->symbol()->location());
                return nullptr;
            }

            new_identifier->type_ref(type_ref);
            if (type_ref->is_unknown_type()) {
                _session.scope_manager()
                    .identifiers_with_unknown_types()
                    .push_back(new_identifier);
            }
        }

        scope->identifiers().add(new_identifier);

        if (init != nullptr
        &&  init->expression()->element_type() == element_type_t::proc_type) {
            if (!add_procedure_instance(
                    context,
                    dynamic_cast<procedure_type*>(init->expression()),
                    source_node)) {
                return nullptr;
            }
        }

        compiler::binary_operator* assign_bin_op = nullptr;

        if (init == nullptr
        &&  init_expr == nullptr
        &&  new_identifier->type_ref() == nullptr) {
            _session.error(
                scope_manager.current_module(),
                "P019",
                fmt::format("unable to infer type: {}", new_identifier->symbol()->name()),
                new_identifier->symbol()->location());
            return nullptr;
        } else {
            if (init == nullptr && init_expr != nullptr) {
                // XXX: revisit after type-widening in binary/unary operators is fixed
                //if (symbol->is_constant()) {
                //    _session.error(
                //        scope_manager.current_module(),
                //        "P028",
                //        "constant variables require constant expressions.",
                //        symbol->location());
                //    return nullptr;
                //}
                assign_bin_op = builder.make_binary_operator(
                    scope,
                    operator_type_t::assignment,
                    builder.make_identifier_reference(
                        scope,
                        new_identifier->symbol()->qualified_symbol(),
                        new_identifier),
                    init_expr);
            }
        }

        return builder.make_declaration(scope, new_identifier, assign_bin_op);
    }

    bool ast_evaluator::nil(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = _session.builder().nil_literal();
        return true;
    }

    bool ast_evaluator::noop(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        return false;
    }

    bool ast_evaluator::symbol(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = _session.builder().make_symbol_from_node(context.node);
        return true;
    }

    bool ast_evaluator::raw_block(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = _session.builder().make_raw_block(
            _session.scope_manager().current_scope(),
            context.node->token.value);
        return true;
    }

    bool ast_evaluator::attribute(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = _session.builder().make_attribute(
            _session.scope_manager().current_scope(),
            context.node->token.value,
            evaluate(context.node->lhs));
        result.element->location(context.node->location);
        return true;
    }

    bool ast_evaluator::directive(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        compiler::element* lhs = nullptr;
        compiler::element* rhs = nullptr;
        compiler::element* body = nullptr;

        element_list_t params {};
        if (context.node->lhs != nullptr) {
            lhs = evaluate(context.node->lhs);
            if (lhs != nullptr) {
                lhs->location(context.node->lhs->location);
                params.emplace_back(lhs);
            }
        }

        if (context.node->rhs != nullptr) {
            rhs = evaluate(context.node->rhs);
            if (rhs != nullptr) {
                rhs->location(context.node->rhs->location);
                params.emplace_back(rhs);
            }
        }

        if (!context.node->children.empty()) {
            auto body_node = context.node->children.front();
            body = evaluate(body_node);
            if (body != nullptr) {
                body->location(body_node->location);
                params.emplace_back(body);
            }
        }

        auto directive_element = builder.make_directive(
            scope_manager.current_scope(),
            context.node->token.value,
            params);
        if (directive_element == nullptr) {
            _session.error(
                _session.scope_manager().current_module(),
                "X000",
                fmt::format("invalid compiler directive: {}", context.node->token.value),
                context.node->location);
            return false;
        }
        directive_element->location(context.node->location);

        result.element = directive_element;

        return true;
    }

    bool ast_evaluator::module(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        auto module = builder.make_module(
            scope_manager.current_scope(),
            builder.make_block(scope_manager.current_scope()));

        result.element = module;

        return true;
    }

    bool ast_evaluator::break_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        compiler::label_reference* label = nullptr;
        if (context.node->lhs != nullptr) {
            label = builder.make_label_reference(
                scope_manager.current_scope(),
                context.node->lhs->token.value);
        }

        result.element = builder.make_break(
            scope_manager.current_scope(),
            label);
        return true;
    }

    bool ast_evaluator::continue_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        compiler::label_reference* label = nullptr;
        if (context.node->lhs != nullptr) {
            label = builder.make_label_reference(
                scope_manager.current_scope(),
                context.node->lhs->token.value);
        }

        result.element = builder.make_continue(
            scope_manager.current_scope(),
            label);
        return true;
    }

    bool ast_evaluator::module_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto expr = resolve_symbol_or_evaluate(context.node->rhs);
        auto mod_ref = _session.builder().make_module_reference(
            _session.scope_manager().current_scope(),
            expr);
        mod_ref->location(context.node->location);

        std::string path;
        if (expr->is_constant() && expr->as_string(path)) {
            boost::filesystem::path module_file(path);
            if (module_file.extension() != ".bc") {
                module_file.append("module.bc");
            }

            boost::filesystem::path source_path = module_file;

            auto current_source_file = _session.current_source_file();
            if (current_source_file != nullptr) {
                if (module_file.is_relative()) {
                    source_path = boost::filesystem::absolute(
                        module_file,
                        current_source_file->path().parent_path());
                }
            }

            auto file_found = true;

            if (!boost::filesystem::exists(source_path)) {
                file_found = false;

                for (const auto& module_path : _session.options().module_paths) {
                    boost::filesystem::path possible_path = module_path;
                    possible_path.append(module_file.string());

                    if (possible_path.is_absolute()) {
                        source_path = possible_path;
                    } else {
                        if (current_source_file != nullptr) {
                            source_path = boost::filesystem::absolute(
                                possible_path,
                                current_source_file->path().parent_path());
                        }
                    }
                    if (boost::filesystem::exists(source_path)) {
                        file_found = true;
                        break;
                    }
                }
            }

            if (!file_found) {
                _session.error(
                    mod_ref->module(),
                    "C021",
                    fmt::format("module file not found: {}", module_file.string()),
                    context.node->location);
                return false;
            }

            auto source_file = _session.add_source_file(source_path);
            auto module = _session.compile_module(source_file);
            if (module == nullptr) {
                _session.error(
                    mod_ref->module(),
                    "C021",
                    "unable to compile module.",
                    context.node->location);
                return false;
            }
            mod_ref->reference(module);
            result.element = mod_ref;
        } else {
            _session.error(
                mod_ref->module(),
                "C021",
                "expected string literal or constant string variable.",
                context.node->rhs->location);
            return false;
        }

        return true;
    }

    bool ast_evaluator::line_comment(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = _session.builder().make_comment(
            _session.scope_manager().current_scope(),
            comment_type_t::line,
            context.node->token.value);
        return true;
    }

    bool ast_evaluator::block_comment(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = _session.builder().make_comment(
            _session.scope_manager().current_scope(),
            comment_type_t::block,
            context.node->token.value);
        return true;
    }

    bool ast_evaluator::string_literal(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = _session.builder().make_string(
            _session.scope_manager().current_scope(),
            context.node->token.value);
        result.element->location(context.node->location);
        return true;
    }

    bool ast_evaluator::number_literal(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        switch (context.node->token.number_type) {
            case syntax::number_types_t::integer: {
                uint64_t value;
                if (context.node->token.parse(value) == syntax::conversion_result_t::success) {
                    if (context.node->token.is_signed()) {
                        result.element = _session.builder().make_integer(
                            _session.scope_manager().current_scope(),
                            common::twos_complement(value));
                    } else {
                        result.element = _session.builder().make_integer(
                            _session.scope_manager().current_scope(),
                            value);
                    }
                    result.element->location(context.node->location);
                    return true;
                } else {
                    _session.error(
                        _session.scope_manager().current_module(),
                        "P041",
                        "invalid integer literal",
                        context.node->location);
                }
                break;
            }
            case syntax::number_types_t::floating_point: {
                double value;
                if (context.node->token.parse(value) == syntax::conversion_result_t::success) {
                    result.element = _session.builder().make_float(
                        _session.scope_manager().current_scope(),
                        value);
                    result.element->location(context.node->location);
                    return true;
                } else {
                    _session.error(
                        _session.scope_manager().current_module(),
                        "P041",
                        "invalid float literal",
                        context.node->location);
                }
                break;
            }
            default:
                break;
        }

        return false;
    }

    bool ast_evaluator::boolean_literal(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto bool_value = context.node->token.as_bool();

        result.element = bool_value ?
            builder.true_literal() :
            builder.false_literal();

        return true;
    }

    bool ast_evaluator::character_literal(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();

        common::rune_t rune = common::rune_invalid;

        if (context.node->token.number_type == syntax::number_types_t::integer) {
            int64_t value;
            if (context.node->token.parse(value) == syntax::conversion_result_t::success) {
                rune = static_cast<common::rune_t>(value);
            } else {
                _session.error(
                    _session.scope_manager().current_module(),
                    "X000",
                    fmt::format("invalid unicode codepoint: {}", context.node->token.value),
                    context.node->location);
                return false;
            }
        } else {
            std::vector<uint8_t> _buffer;
            for (auto c : context.node->token.value)
                _buffer.push_back(c);

            if (!_buffer.empty()) {
                auto ch = _buffer[0];
                rune = ch;
                if (ch >= 0x80) {
                    auto cp = common::utf8_decode(
                        (char*) (_buffer.data()),
                        _buffer.size());
                    rune = cp.value;
                }
            }
        }

        result.element = builder.make_character(
            _session.scope_manager().current_scope(),
            rune);

        return true;
    }

    bool ast_evaluator::namespace_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = _session.builder().make_namespace(
            _session.scope_manager().current_scope(),
            evaluate(context.node->rhs));
        return true;
    }

    bool ast_evaluator::expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = _session.builder().make_expression(
            _session.scope_manager().current_scope(),
            evaluate(context.node->lhs));
        result.element->location(context.node->location);
        return true;
    }

    bool ast_evaluator::argument_list(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        auto args = builder.make_argument_list(scope_manager.current_scope());
        for (auto arg_node : context.node->children) {
            compiler::element* arg = nullptr;

            switch (arg_node->type) {
                case syntax::ast_node_type_t::assignment: {
                    auto lhs = evaluate(arg_node->lhs->children.front());
                    auto rhs = resolve_symbol_or_evaluate(arg_node->rhs->children.front());
                    if (lhs != nullptr && rhs != nullptr)
                        arg = builder.make_argument_pair(
                            scope_manager.current_scope(),
                            lhs,
                            rhs);
                    break;
                }
                default: {
                    arg = resolve_symbol_or_evaluate(arg_node);
                    break;
                }
            }

            if (arg == nullptr) {
                // XXX: error
                return false;
            }

            arg->location(arg_node->location);
            args->add(arg);
            arg->parent_element(args);
        }

        args->location(context.node->location);

        result.element = args;

        return true;
    }

    bool ast_evaluator::unary_operator(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto it = s_unary_operators.find(context.node->token.type);
        if (it == s_unary_operators.end())
            return false;
        result.element = _session.builder().make_unary_operator(
            _session.scope_manager().current_scope(),
            it->second,
            resolve_symbol_or_evaluate(context.node->rhs));
        return true;
    }

    bool ast_evaluator::spread_operator(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        compiler::element* expr = nullptr;
        if (context.node->lhs != nullptr) {
            expr = resolve_symbol_or_evaluate(context.node->lhs);
        }
        result.element = _session.builder().make_spread_operator(
            _session.scope_manager().current_scope(),
            expr);
        return true;
    }

    bool ast_evaluator::binary_operator(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto it = s_binary_operators.find(context.node->token.type);
        if (it == s_binary_operators.end())
            return false;
        auto scope = _session.scope_manager().current_scope();
        auto& builder = _session.builder();

        auto is_member_access = it->second == operator_type_t::member_access;

        compiler::element* lhs = nullptr;
        compiler::element* rhs = nullptr;

        if (is_logical_conjunction_operator(it->second)) {
            lhs = convert_predicate(context, context.node->lhs, scope);
            rhs = convert_predicate(context, context.node->rhs, scope);
        } else {
            lhs = resolve_symbol_or_evaluate(context.node->lhs);
            if (lhs == nullptr) {
                _session.error(
                    _session.scope_manager().current_module(),
                    "P052",
                    "unknown identifier.",
                    context.node->lhs->location);
                return false;
            }

            compiler::block* type_scope = nullptr;
            infer_type_result_t infer_type_result {};
            if (lhs->infer_type(_session, infer_type_result)) {
                if (infer_type_result.inferred_type->is_composite_type()) {
                    compiler::composite_type* composite_type = nullptr;
                    if (infer_type_result.inferred_type->is_pointer_type()) {
                        auto pointer_type = dynamic_cast<compiler::pointer_type*>(infer_type_result.inferred_type);
                        composite_type = dynamic_cast<compiler::composite_type*>(pointer_type->base_type_ref()->type());
                        if (is_member_access) {
                            auto location = lhs->location();
                            lhs = builder.make_unary_operator(
                                scope,
                                operator_type_t::pointer_dereference,
                                lhs);
                            lhs->location(location);
                        }
                    } else {
                        composite_type = dynamic_cast<compiler::composite_type*>(infer_type_result.inferred_type);
                    }

                    type_scope = composite_type->scope();
                }
            }

            rhs = resolve_symbol_or_evaluate(
                context.node->rhs,
                type_scope);
        }

        auto bin_op = builder.make_binary_operator(
            scope,
            it->second,
            lhs,
            rhs);

        common::source_location loc {};
        loc.start(lhs->location().start());
        loc.end(rhs->location().end());
        bin_op->location(loc);

        result.element = bin_op;

        return true;
    }

    bool ast_evaluator::while_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();
        auto scope = scope_manager.current_scope();

        auto predicate = convert_predicate(
            context,
            context.node->lhs,
            scope);
        auto body = evaluate(context.node->rhs);

        result.element = builder.make_while(
            scope,
            dynamic_cast<compiler::binary_operator*>(predicate),
            dynamic_cast<compiler::block*>(body));

        return true;
    }

    bool ast_evaluator::return_statement(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto return_element = _session.builder().make_return(_session.scope_manager().current_scope());
        auto& expressions = return_element->expressions();
        for (auto arg_node : context.node->rhs->children) {
            auto arg = resolve_symbol_or_evaluate(arg_node);
            expressions.push_back(arg);
            arg->parent_element(return_element);
        }
        result.element = return_element;
        return true;
    }

    bool ast_evaluator::import_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        qualified_symbol_t qualified_symbol {};
        builder.make_qualified_symbol(qualified_symbol, context.node->lhs);

        compiler::identifier_reference* from_ref = nullptr;
        if (context.node->rhs != nullptr) {
            from_ref = dynamic_cast<compiler::identifier_reference*>(
                resolve_symbol_or_evaluate(context.node->rhs));
            qualified_symbol.namespaces.insert(
                qualified_symbol.namespaces.begin(),
                from_ref->symbol().name);
        }

        auto vars = scope_manager.find_identifier(qualified_symbol);
        compiler::identifier* identifier = vars.empty() ? nullptr : vars.front();

        compiler::module_reference* mod_ref = nullptr;
        auto symbol_ref = builder.make_identifier_reference(
            scope_manager.current_scope(),
            qualified_symbol,
            identifier);
        if (from_ref != nullptr) {
            // XXX: handle case where module reference not found
            auto var = from_ref->identifier();
            if (var != nullptr) {
                mod_ref = dynamic_cast<compiler::module_reference*>(var->initializer()->expression());
            }
        }

        auto import = builder.make_import(
            scope_manager.current_scope(),
            symbol_ref,
            from_ref,
            mod_ref);
        add_expression_to_scope(scope_manager.current_scope(), import);

        result.element = import;

        return true;
    }

    bool ast_evaluator::subscript_operator(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        compiler::element* lhs = nullptr;
        compiler::element* rhs = nullptr;
        compiler::integer_literal* place_holder = nullptr;

        element_list_t rhs_list {};
        auto current_subscript = context.node;
        while (current_subscript->type == syntax::ast_node_type_t::subscript_operator) {
            rhs_list.emplace_back(resolve_symbol_or_evaluate(current_subscript->rhs));
            current_subscript = current_subscript->lhs;
        }
        lhs = resolve_symbol_or_evaluate(current_subscript);

        if (rhs_list.size() == 1) {
            rhs = rhs_list.back();
        } else {
            place_holder = builder.make_integer(scope_manager.current_scope(), 0);
            rhs = builder.make_binary_operator(
                scope_manager.current_scope(),
                operator_type_t::add,
                rhs_list.front(),
                builder.make_binary_operator(
                    scope_manager.current_scope(),
                    operator_type_t::multiply,
                    rhs_list.back(),
                    place_holder));
        }

        auto member_access = builder.make_binary_operator(
            scope_manager.current_scope(),
            operator_type_t::member_access,
            lhs,
            builder.make_identifier_reference(
                scope_manager.current_scope(),
                qualified_symbol_t("data"),
                nullptr));

        result.element = builder.make_binary_operator(
            scope_manager.current_scope(),
            operator_type_t::subscript,
            member_access,
            rhs);

        if (place_holder != nullptr) {
            auto& ids = result.element->ids();
            ids.insert(place_holder->id());
        }

        common::source_location loc;
        loc.start(lhs->location().start());
        loc.end(rhs->location().end());
        result.element->location(loc);

        return true;
    }

    bool ast_evaluator::basic_block(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& scope_manager = _session.scope_manager();
        auto active_scope = scope_manager.push_new_block();

        for (auto current_node : context.node->children) {
            auto expr = evaluate(current_node);
            if (expr == nullptr) {
                _session.error(
                    _session.scope_manager().current_module(),
                    "C024",
                    "invalid statement",
                    current_node->location);
                return false;
            }
            add_expression_to_scope(active_scope, expr);
            expr->parent_element(active_scope);
        }

        result.element = scope_manager.pop_scope();

        if (context.node->has_attribute("parent_scope")) {
            auto current_scope = scope_manager.current_scope();
            for (auto identifier : active_scope->identifiers().as_list())
                current_scope->identifiers().add(identifier);
        }

        return true;
    }

    bool ast_evaluator::proc_call(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        auto is_ufcs = context.node->ufcs;

        compiler::argument_list* args = nullptr;
        auto argument_list = evaluate(context.node->rhs);
        if (argument_list == nullptr)
            return false;

        args = dynamic_cast<compiler::argument_list*>(argument_list);
        if (is_ufcs) {
            const auto& elements = args->elements();
            auto self_arg = elements[0];
            infer_type_result_t type_result {};
            if (!self_arg->infer_type(_session, type_result))
                return false;
            if (!type_result.inferred_type->is_pointer_type()) {
                auto address_of_args = builder.make_argument_list(args->parent_scope());
                address_of_args->add(self_arg);
                self_arg->parent_element(address_of_args);

                auto address_of_call = compiler::intrinsic::intrinsic_for_call(
                    _session,
                    args->parent_scope(),
                    address_of_args,
                    qualified_symbol_t("address_of"),
                    {});
                args->replace(0, address_of_call);
                address_of_call->parent_element(args);
            }
        }

        auto symbol_node = context.node->lhs->rhs;

        qualified_symbol_t proc_name {};
        builder.make_qualified_symbol(proc_name, symbol_node);

        auto type_params = builder.make_tagged_type_list_from_node(symbol_node);

        auto intrinsic = compiler::intrinsic::intrinsic_for_call(
            _session,
            scope_manager.current_scope(),
            args,
            proc_name,
            type_params);
        if (intrinsic != nullptr) {
            intrinsic->uniform_function_call(is_ufcs);
            result.element = intrinsic;
            return true;
        }

        compiler::identifier_reference_list_t references {};
        auto vars = scope_manager.find_identifier(proc_name);
        if (vars.empty()) {
            auto unresolved = builder.make_identifier_reference(
                scope_manager.current_scope(),
                proc_name,
                nullptr);
            references.emplace_back(unresolved);
        } else {
            for (auto proc_identifier : vars) {
                references.emplace_back(builder.make_identifier_reference(
                    scope_manager.current_scope(),
                    proc_name,
                    proc_identifier));
            }
        }

        auto proc_call = builder.make_procedure_call(
            scope_manager.current_scope(),
            args,
            type_params,
            references);
        proc_call->uniform_function_call(is_ufcs);
        proc_call->location(symbol_node->location);
        for (auto ref : proc_call->references())
            ref->parent_element(proc_call);
        result.element = proc_call;

        return true;
    }

    bool ast_evaluator::statement(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        label_list_t labels {};

        for (auto label : context.node->labels) {
            labels.push_back(builder.make_label(
                scope_manager.current_scope(),
                label->token.value));
        }

        compiler::element* expr = nullptr;
        if (context.node->rhs != nullptr) {
            expr = evaluate(context.node->rhs);
            if (expr == nullptr)
                return false;

            switch (expr->element_type()) {
                case element_type_t::block: {
                    auto block = dynamic_cast<compiler::block*>(expr);
                    block->has_stack_frame(true);
                    break;
                }
                case element_type_t::symbol: {
                    auto e = evaluate(context.node->rhs->rhs);
                    auto type_ref = dynamic_cast<compiler::type_reference*>(e);
                    if (type_ref == nullptr) {
                        _session.error(
                            _session.scope_manager().current_module(),
                            "X000",
                            "invalid type expression.",
                            context.node->rhs->rhs->location);
                        return false;
                    }
                    expr = add_identifier_to_scope(
                        context,
                        dynamic_cast<compiler::symbol_element*>(expr),
                        type_ref,
                        nullptr,
                        0);
                    break;
                }
                default: {
                    break;
                }
            }
        }

        result.element = builder.make_statement(
            scope_manager.current_scope(),
            labels,
            expr);

        return true;
    }

    bool ast_evaluator::cast_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        compiler::argument_list* args = nullptr;
        auto argument_list = evaluate(context.node->rhs);
        if (argument_list == nullptr)
            return false;
        args = dynamic_cast<compiler::argument_list*>(argument_list);

        auto symbol_node = context.node->lhs->rhs;

        qualified_symbol_t proc_name {};
        builder.make_qualified_symbol(proc_name, symbol_node);

        auto type_params = builder.make_tagged_type_list_from_node(symbol_node);

        // XXX: should cast accept an argument_list?
        auto cast_element = builder.make_cast(
            scope_manager.current_scope(),
            type_params[0],
            args->elements().front());
        cast_element->location(context.node->location);
        cast_element->type_location(context.node->lhs->rhs->location);
        result.element = cast_element;

        return true;
    }

    bool ast_evaluator::transmute_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        compiler::argument_list* args = nullptr;
        auto argument_list = evaluate(context.node->rhs);
        if (argument_list == nullptr)
            return false;
        args = dynamic_cast<compiler::argument_list*>(argument_list);

        auto symbol_node = context.node->lhs->rhs;

        qualified_symbol_t proc_name {};
        builder.make_qualified_symbol(proc_name, symbol_node);

        auto type_params = builder.make_tagged_type_list_from_node(symbol_node);

        // XXX: like cast, should transmute support argument_list directly?
        auto transmute_element = builder.make_transmute(
            scope_manager.current_scope(),
            type_params[0],
            args->elements().front());
        transmute_element->location(context.node->location);
        transmute_element->type_location(context.node->lhs->rhs->location);
        result.element = transmute_element;

        return true;
    }

    bool ast_evaluator::new_literal(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        compiler::argument_list* args = nullptr;
        auto argument_list = evaluate(context.node->rhs);
        if (argument_list == nullptr)
            return false;
        args = dynamic_cast<compiler::argument_list*>(argument_list);

        auto symbol_node = context.node->lhs->rhs;

        qualified_symbol_t proc_name {};
        builder.make_qualified_symbol(proc_name, symbol_node);

        auto type_params = builder.make_tagged_type_list_from_node(symbol_node);

        auto literal = builder.make_user_literal(
            scope_manager.current_scope(),
            type_params[0],
            type_params,
            args);
        literal->location(context.node->location);
        result.element = literal;

        return true;
    }

    bool ast_evaluator::tuple_literal(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        auto symbol_node = context.node->lhs->rhs;

        qualified_symbol_t proc_name {};
        builder.make_qualified_symbol(proc_name, symbol_node);

        auto literal = make_tuple_literal(
            context,
            scope_manager.current_scope(),
            builder.make_tagged_type_list_from_node(symbol_node));
        literal->location(context.node->location);
        result.element = literal;

        return true;
    }

    bool ast_evaluator::array_literal(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        compiler::argument_list* args = nullptr;
        auto argument_list = evaluate(context.node->rhs);
        if (argument_list == nullptr)
            return false;
        args = dynamic_cast<compiler::argument_list*>(argument_list);

        auto symbol_node = context.node->lhs->rhs;

        qualified_symbol_t proc_name {};
        builder.make_qualified_symbol(proc_name, symbol_node);

        auto type_params = builder.make_tagged_type_list_from_node(symbol_node);

        element_list_t subscripts {};
        if (args != nullptr) {
            subscripts.push_back(builder.make_integer(
                scope_manager.current_scope(),
                args->size()));
        }

        auto array_type = scope_manager.find_array_type(
            type_params[0]->type(),
            subscripts);
        if (array_type == nullptr) {
            array_type = builder.make_array_type(
                scope_manager.current_scope(),
                builder.make_block(scope_manager.current_scope()),
                type_params,
                subscripts);
        }
        auto type_ref = builder.make_type_reference(
            scope_manager.current_scope(),
            qualified_symbol_t(),
            array_type);

        auto literal = builder.make_array_literal(
            scope_manager.current_scope(),
            type_ref,
            type_params,
            args,
            subscripts);
        literal->location(context.node->location);
        result.element = literal;

        return true;
    }

    bool ast_evaluator::enum_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        auto active_scope = scope_manager.current_scope();
        auto enum_scope = builder.make_block(active_scope);
        auto enum_type = builder.make_enum_type(active_scope, enum_scope);
        enum_type->location(context.node->location);
        active_scope->types().add(enum_type);

        add_type_parameters(
            context,
            enum_scope,
            context.node->lhs,
            enum_type->type_parameters());

        auto success = add_composite_type_fields(
            context,
            enum_type,
            context.node->rhs);
        if (!success)
            return false;

        if (!enum_type->initialize(_session))
            return false;

        result.element = enum_type;
        return true;
    }

    bool ast_evaluator::case_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        compiler::element* expr = nullptr;
        if (context.node->lhs != nullptr)
            expr = evaluate(context.node->lhs);
        auto scope = dynamic_cast<compiler::block*>(evaluate(context.node->rhs));

        result.element = builder.make_case(
            scope_manager.current_scope(),
            scope,
            expr);

        return true;
    }

    bool ast_evaluator::with_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        result.element = builder.make_with(
            scope_manager.current_scope(),
            resolve_symbol_or_evaluate(context.node->lhs),
            dynamic_cast<compiler::block*>(evaluate(context.node->rhs)));
        return true;
    }

    bool ast_evaluator::family_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        type_reference_list_t types {};
        for (auto type_node : context.node->rhs->children) {
            auto type_ref = dynamic_cast<compiler::type_reference*>(evaluate_in_scope(
                type_node,
                scope_manager.current_scope()));
            if (type_ref->is_unknown_type()) {
                // XXX: need to extend session support this
                //      scenario.
            }
            types.emplace_back(type_ref);
        }

        result.element = builder.make_family_type(
            scope_manager.current_scope(),
            types);

        return true;
    }

    bool ast_evaluator::switch_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        auto expr = resolve_symbol_or_evaluate(context.node->lhs);
        auto scope = dynamic_cast<compiler::block*>(evaluate(context.node->rhs));

        result.element = builder.make_switch(
            scope_manager.current_scope(),
            scope,
            expr);

        return true;
    }

    bool ast_evaluator::defer_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        auto defer_e = builder.make_defer(
            scope_manager.current_scope(),
            evaluate(context.node->lhs));
        scope_manager.current_scope()->defer_stack().push(defer_e);

        result.element = defer_e;

        return true;
    }

    bool ast_evaluator::struct_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        auto active_scope = scope_manager.current_scope();
        auto struct_scope = builder.make_block(active_scope);

        auto struct_type = builder.make_struct_type(
            active_scope,
            struct_scope);
        active_scope->types().add(struct_type);

        add_type_parameters(
            context,
            struct_scope,
            context.node->lhs,
            struct_type->type_parameters());

        auto success = add_composite_type_fields(
            context,
            struct_type,
            context.node->rhs);
        if (!success)
            return false;

        if (!struct_type->initialize(_session))
            return false;

        result.element = struct_type;
        return true;
    }

    bool ast_evaluator::union_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        auto active_scope = scope_manager.current_scope();
        auto union_scope = builder.make_block(active_scope);
        auto union_type = builder.make_union_type(active_scope, union_scope);
        active_scope->types().add(union_type);

        add_type_parameters(
            context,
            union_scope,
            context.node->lhs,
            union_type->type_parameters());

        auto success = add_composite_type_fields(
            context,
            union_type,
            context.node->rhs);
        if (!success)
            return false;

        if (!union_type->initialize(_session))
            return false;

        result.element = union_type;
        return true;
    }

    bool ast_evaluator::else_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = evaluate(context.node->children[0]);
        return true;
    }

    bool ast_evaluator::if_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto scope = _session.scope_manager().current_scope();

        auto predicate = convert_predicate(context, context.node->lhs, scope);
        auto true_branch = evaluate(context.node->children[0]);
        auto false_branch = evaluate(context.node->rhs);
        result.element = _session.builder().make_if(
            scope,
            predicate,
            true_branch,
            false_branch,
            context.node->type == syntax::ast_node_type_t::elseif_expression);
        return true;
    }

    bool ast_evaluator::proc_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        auto active_scope = scope_manager.current_scope();
        auto block_scope = builder.make_block(active_scope);
        auto proc_type = builder.make_procedure_type(active_scope, block_scope);
        proc_type->location(context.node->location);
        active_scope->types().add(proc_type);

        add_type_parameters(
            context,
            block_scope,
            context.node->lhs->lhs,
            proc_type->type_parameters());

        add_procedure_type_return_field(
            context,
            proc_type,
            block_scope,
            context.node->lhs->rhs);

        if (!add_procedure_type_parameter_fields(
                context,
                proc_type,
                block_scope,
                context.node->rhs)) {
            return false;
        }

        result.element = proc_type;

        return true;
    }

    bool ast_evaluator::lambda_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        auto active_scope = scope_manager.current_scope();
        auto block_scope = builder.make_block(active_scope);
        auto proc_type = builder.make_procedure_type(active_scope, block_scope);
        active_scope->types().add(proc_type);

        add_type_parameters(
            context,
            block_scope,
            context.node->lhs->lhs,
            proc_type->type_parameters());

        add_procedure_type_return_field(
            context,
            proc_type,
            block_scope,
            context.node->lhs->rhs);

        auto open_generic_type = scope_manager.find_generic_type({});
        context.decl_type_ref = builder.make_type_reference(
            block_scope,
            open_generic_type->name(),
            open_generic_type);

        if (!add_procedure_type_parameter_fields(
                context,
                proc_type,
                block_scope,
                context.node->rhs)) {
            return false;
        }

        result.element = proc_type;

        return true;
    }

    bool ast_evaluator::assignment(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto assignment = _session
            .builder()
            .make_assignment(_session.scope_manager().current_scope());
        auto success = add_assignments_to_scope(
            context,
            context.node,
            assignment->expressions(),
            nullptr);
        for (auto expr : assignment->expressions())
            expr->parent_element(assignment);
        result.element = assignment;
        return success;
    }

    bool ast_evaluator::type_declaration(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();
        auto scope = scope_manager.current_scope();

        compiler::type_reference* type_decl_ref = nullptr;

        std::stack<syntax::ast_node_t*> type_nodes {};
        auto current = context.node->lhs;
        while (true) {
            type_nodes.push(current);
            if (current->type == syntax::ast_node_type_t::struct_expression
            ||  current->type == syntax::ast_node_type_t::union_expression
            ||  current->type == syntax::ast_node_type_t::enum_expression) {
                break;
            }
            if (current->rhs == nullptr)
                break;
            current = current->rhs;
        }

        compiler::element_list_t subscripts {};
        while (!type_nodes.empty()) {
            current = type_nodes.top();
            type_nodes.pop();

            switch (current->type) {
                case syntax::ast_node_type_t::symbol:
                case syntax::ast_node_type_t::type_tagged_symbol: {
                    qualified_symbol_t type_name{};
                    builder.make_qualified_symbol(
                        type_name,
                        current);
                    auto symbol = builder.make_symbol_from_node(current, scope);
                    auto type = scope_manager.find_type(type_name, scope);
                    if (type == nullptr) {
                        type = builder.make_unknown_type(scope, symbol);
                    }
                    type_decl_ref = builder.make_type_reference(
                        scope,
                        type->symbol()->qualified_symbol(),
                        type);
                    break;
                }
                case syntax::ast_node_type_t::enum_expression:
                case syntax::ast_node_type_t::union_expression:
                case syntax::ast_node_type_t::struct_expression: {
                    auto type = dynamic_cast<compiler::type*>(evaluate(current));
                    if (type == nullptr) {
                        return false;
                    }
                    type_decl_ref = builder.make_type_reference(
                        scope,
                        type->symbol()->qualified_symbol(),
                        type);
                    break;
                }
                case syntax::ast_node_type_t::pointer_declaration: {
                    auto type = scope_manager.find_pointer_type(
                        type_decl_ref->type(),
                        scope);
                    if (type == nullptr) {
                        type = builder.make_pointer_type(
                            scope,
                            type_decl_ref->type()->symbol()->qualified_symbol(),
                            type_decl_ref->type());
                    }
                    type_decl_ref = builder.make_type_reference(
                        scope,
                        qualified_symbol_t(),
                        type);
                    break;
                }
                case syntax::ast_node_type_t::subscript_declaration: {
                    auto is_dynamic = current->lhs == nullptr;
                    if (!is_dynamic) {
                        subscripts.push_back(resolve_symbol_or_evaluate(current->lhs));
                    }

                    while (!type_nodes.empty()) {
                        auto subscript_node = type_nodes.top();
                        if (subscript_node->type != syntax::ast_node_type_t::subscript_declaration)
                            break;

                        if (is_dynamic) {
                            // XXX: error
                            return false;
                        }

                        subscripts.push_back(resolve_symbol_or_evaluate(subscript_node->lhs));

                        type_nodes.pop();
                    }

                    auto type = scope_manager.find_array_type(
                        type_decl_ref->type(),
                        subscripts,
                        scope);
                    if (type == nullptr) {
                        type = builder.make_array_type(
                            scope,
                            builder.make_block(scope),
                            {type_decl_ref},
                            subscripts);
                    }
                    type_decl_ref = builder.make_type_reference(
                        scope,
                        qualified_symbol_t(),
                        type);
                    break;
                }
                default: {
                    // XXX: error
                    return false;
                }
            }
        }

        result.element = type_decl_ref;

        return true;
    }

    bool ast_evaluator::for_in_statement(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        auto rhs = resolve_symbol_or_evaluate(context.node->rhs);
        if (rhs == nullptr) {
            // XXX: error
            return false;
        }

        auto lhs = evaluate(context.node->lhs);
        if (lhs == nullptr || lhs->element_type() != element_type_t::symbol) {
            // XXX: error
            return false;
        }

        compiler::type_reference* type_ref = nullptr;
        if (context.node->lhs->rhs != nullptr) {
            auto ref = evaluate(context.node->lhs->rhs);
            type_ref = dynamic_cast<compiler::type_reference*>(ref);
        } else {
            infer_type_result_t infer_type_result {};
            if (rhs->infer_type(_session, infer_type_result)) {
                type_ref = infer_type_result.reference;
                if (type_ref == nullptr) {
                    type_ref = builder.make_type_reference(
                        scope_manager.current_scope(),
                        infer_type_result.inferred_type->symbol()->qualified_symbol(),
                        infer_type_result.inferred_type);
                }
            } else {
                // XXX: error
                return false;
            }
        }

        auto for_scope = builder.make_block(scope_manager.current_scope());
        scope_manager.current_scope()->blocks().emplace_back(for_scope);

        auto induction_decl = add_identifier_to_scope(
            context,
            dynamic_cast<compiler::symbol_element*>(lhs),
            type_ref,
            nullptr,
            0,
            for_scope);

        auto block = evaluate_in_scope(
            context.node->children.front(),
            for_scope);
        auto body = dynamic_cast<compiler::block*>(block);

        result.element = builder.make_for(
            scope_manager.current_scope(),
            induction_decl,
            rhs,
            body);
        for_scope->parent_element(result.element);
        for_scope->has_stack_frame(true);

        return true;
    }

    bool ast_evaluator::with_member_access(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto member_access_bin_op = _session.ast_builder().binary_operator_node(
            context.node->lhs,
            syntax::s_period_literal,
            context.node->rhs);
        context.node = member_access_bin_op;
        return binary_operator(context, result);
    }

    bool ast_evaluator::fallthrough_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = _session
            .builder()
            .make_fallthrough(
                _session.scope_manager().current_scope(),
                nullptr);
        return true;
    }

    bool ast_evaluator::add_assignments_to_scope(
            const evaluator_context_t& context,
            const syntax::ast_node_t* node,
            element_list_t& expressions,
            compiler::block* scope) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        auto target_list = node->lhs;
        auto source_list = node->rhs;

        const bool is_constant_assignment = node->type == syntax::ast_node_type_t::constant_assignment;

        if (target_list->children.size() != source_list->children.size()) {
            _session.error(
                _session.scope_manager().current_module(),
                "P027",
                "the number of left-hand-side targets must match"
                " the number of right-hand-side expressions.",
                source_list->location);
            return false;
        }

        for (size_t i = 0; i < target_list->children.size(); i++) {
            auto target_symbol = target_list->children[i];

            auto is_binary_op = true;
            auto target_element = resolve_symbol_or_evaluate(
                target_symbol,
                scope,
                false);
            if (target_element == nullptr) {
                _session.error(
                    _session.scope_manager().current_module(),
                    "X000",
                    "unable to evaluate target element.",
                    target_symbol->location);
                return false;
            }

            if (target_element->element_type() == element_type_t::identifier_reference) {
                // XXX: clean this up
                //
                auto identifier_ref = dynamic_cast<compiler::identifier_reference*>(target_element);
                if (identifier_ref->resolved()
                &&  !identifier_ref->identifier()->type_ref()->is_proc_type()) {
                    // XXX: it makes sense that we only want to check for reassignment here
                    //      if it's constant AND we're adding something to the same scope
                    if (identifier_ref->is_constant()) {
                        auto current_scope = scope != nullptr ? scope : scope_manager.current_scope();
                        if (current_scope->id() == identifier_ref->parent_scope()->id()) {
                            _session.error(
                                _session.scope_manager().current_module(),
                                "P028",
                                "constant variables cannot be modified.",
                                target_symbol->location);
                            return false;
                        } else {
                            is_binary_op = false;
                        }
                    }
                } else {
                    is_binary_op = false;
                }
            }

            if (is_binary_op) {
                auto rhs = resolve_symbol_or_evaluate(
                    source_list->children[i],
                    scope);
                if (rhs == nullptr)
                    return false;

                auto binary_op = builder.make_binary_operator(
                    scope_manager.current_scope(),
                    operator_type_t::assignment,
                    target_element,
                    rhs);
                expressions.emplace_back(binary_op);
            } else {
                _session.elements().remove(target_element->id());

                auto lhs = evaluate_in_scope(
                    target_symbol,
                    scope);
                if (lhs == nullptr)
                    return false;

                auto symbol = dynamic_cast<compiler::symbol_element*>(lhs);
                symbol->constant(is_constant_assignment);

                auto type_ref = dynamic_cast<compiler::type_reference*>(evaluate_in_scope(
                    target_symbol->rhs,
                    scope));
                auto decl = add_identifier_to_scope(
                    context,
                    symbol,
                    type_ref,
                    node,
                    i,
                    scope);
                if (decl == nullptr)
                    return false;

                expressions.emplace_back(decl);
            }
        }

        return true;
    }

    bool ast_evaluator::add_procedure_type_parameter_fields(
            evaluator_context_t& context,
            compiler::procedure_type* proc_type,
            compiler::block* block_scope,
            const syntax::ast_node_t* parameters_node) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        compiler::field* param_field = nullptr;
        auto& parameter_map = proc_type->parameters();

        auto add_param_decl = [&](compiler::declaration* param_decl, bool is_variadic) {
            param_decl->identifier()->usage(identifier_usage_t::stack);
            param_field = builder.make_field(
                proc_type,
                block_scope,
                param_decl,
                param_field != nullptr ? param_field->end_offset() : 0,
                0,
                is_variadic);
            parameter_map.add(param_field);
            param_decl->identifier()->field(param_field);
        };

        syntax::ast_node_t* current_type = nullptr;
        syntax::ast_node_list type_nodes {};
        type_nodes.resize(parameters_node->children.size());

        size_t index = parameters_node->children.empty() ? 0 : parameters_node->children.size() - 1;
        for (auto it = parameters_node->children.rbegin();
                it != parameters_node->children.rend();
                ++it) {
            auto param_node = *it;
            if (param_node->rhs != nullptr
            &&  param_node->rhs->type == syntax::ast_node_type_t::type_declaration)
                current_type = param_node->rhs;
            type_nodes[index--] = current_type;
        }

        index = 0;
        for (auto param_node : parameters_node->children) {
            switch (param_node->type) {
                case syntax::ast_node_type_t::symbol: {
                    if (param_node->rhs == nullptr) {
                        context.decl_type_ref = dynamic_cast<compiler::type_reference*>(evaluate(type_nodes[index]));
                    }

                    auto param_decl = declare_identifier(
                        context,
                        param_node,
                        block_scope);
                    if (param_decl == nullptr) {
                        _session.error(
                            scope_manager.current_module(),
                            "X000",
                            "invalid procedure parameter declaration.",
                            param_node->location);
                        return false;
                    }

                    add_param_decl(param_decl, false);
                    context.decl_type_ref = nullptr;

                    break;
                }
                case syntax::ast_node_type_t::assignment: {
                    element_list_t list {};
                    auto success = add_assignments_to_scope(
                        context,
                        param_node,
                        list,
                        block_scope);
                    if (!success) {
                        _session.error(
                            scope_manager.current_module(),
                            "X000",
                            "invalid procedure parameter declaration via assignment.",
                            param_node->location);
                        return false;
                    }

                    auto param_decl = dynamic_cast<compiler::declaration*>(list.front());
                    add_param_decl(param_decl, false);

                    break;
                }
                case syntax::ast_node_type_t::spread_operator: {
                    auto param_decl = declare_identifier(
                        context,
                        param_node->rhs,
                        block_scope);
                    if (param_decl == nullptr) {
                        _session.error(
                            scope_manager.current_module(),
                            "X000",
                            "invalid procedure variadic parameter declaration.",
                            param_node->location);
                        return false;
                    }

                    if (index != parameters_node->children.size() - 1) {
                        _session.error(
                            _session.scope_manager().current_module(),
                            "P019",
                            fmt::format(
                                "variadic parameter only valid in final position: {}",
                                param_decl->identifier()->symbol()->name()),
                            proc_type->location());
                        return false;
                    }

                    add_param_decl(param_decl, true);

                    break;
                }
                default: {
                    return false;
                }
            }
            ++index;
        }

        return true;
    }

    void ast_evaluator::add_procedure_type_return_field(
            const evaluator_context_t& context,
            compiler::procedure_type* proc_type,
            compiler::block* block_scope,
            const syntax::ast_node_t* return_type_node) {
        if (return_type_node == nullptr)
            return;
        auto& builder = _session.builder();
        auto return_identifier = builder.make_identifier(
            block_scope,
            builder.make_symbol(block_scope, "_retval"),
            nullptr);
        return_identifier->usage(identifier_usage_t::stack);

        auto type_ref = dynamic_cast<compiler::type_reference*>(evaluate_in_scope(
            return_type_node,
            block_scope));
        if (type_ref->is_unknown_type()) {
            _session.scope_manager()
                .identifiers_with_unknown_types()
                .push_back(return_identifier);
        }
        return_identifier->type_ref(type_ref);
        auto return_field = builder.make_field(
            proc_type,
            block_scope,
            builder.make_declaration(block_scope, return_identifier, nullptr),
            0);
        proc_type->return_type(return_field);
        return_identifier->field(return_field);
    }

    compiler::declaration* ast_evaluator::declare_identifier(
            const evaluator_context_t& context,
            const syntax::ast_node_t* node,
            compiler::block* scope) {
        auto identifier = dynamic_cast<compiler::symbol_element*>(evaluate_in_scope(
            node,
            scope));

        auto declared_type_ref = dynamic_cast<compiler::type_reference*>(evaluate_in_scope(
            node->rhs,
            scope));
        auto type_ref = declared_type_ref != nullptr ?
            declared_type_ref :
            context.decl_type_ref;
        if (type_ref == nullptr)
            return nullptr;

        return add_identifier_to_scope(
            context,
            identifier,
            type_ref,
            nullptr,
            0,
            scope);
    }

    element* ast_evaluator::convert_predicate(
            const evaluator_context_t& context,
            const syntax::ast_node_t* node,
            compiler::block* scope) {
        auto& builder = _session.builder();

        auto predicate = resolve_symbol_or_evaluate(node, scope);
        if (predicate->element_type() != element_type_t::binary_operator) {
            infer_type_result_t infer_type_result {};
            if (!predicate->infer_type(_session, infer_type_result))
                return nullptr;

            if (infer_type_result.inferred_type->element_type() != element_type_t::bool_type) {
                _session.error(
                    _session.scope_manager().current_module(),
                    "P002",
                    "expected a boolean expression.",
                    predicate->location());
                return nullptr;
            }

            predicate = builder.make_binary_operator(
                scope,
                operator_type_t::equals,
                predicate,
                builder.true_literal());
        }

        return predicate;
    }

    bool ast_evaluator::uninitialized_literal(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        result.element = builder.uninitialized_literal();
        return true;
    }

    compiler::type_literal* ast_evaluator::make_tuple_literal(
            const evaluator_context_t& context,
            compiler::block* scope,
            const compiler::type_reference_list_t& type_params) {
        auto& builder = _session.builder();
        auto& ast_builder = _session.ast_builder();
        auto& scope_manager = _session.scope_manager();

        auto active_scope = scope != nullptr ? scope : scope_manager.current_scope();

        auto type_scope = builder.make_block(active_scope);
        auto tuple_type = builder.make_tuple_type(active_scope, type_scope);
        active_scope->types().add(tuple_type);

        // XXX: should mixed named and unnamed fields be allowed?
        auto args = builder.make_argument_list(active_scope);

        size_t index = 0;
        compiler::field* previous_field = nullptr;
        for (auto arg : context.node->rhs->children) {
            syntax::ast_node_t* assignment_node = nullptr;

            if (arg->type != syntax::ast_node_type_t::assignment) {
                syntax::token_t field_name;
                field_name.type = syntax::token_types_t::identifier;
                field_name.value = fmt::format("_{}", index);

                auto field_symbol = ast_builder.symbol_node();
                field_symbol->children.push_back(ast_builder.symbol_part_node(field_name));

                assignment_node = ast_builder.assignment_node();
                assignment_node->lhs->children.push_back(field_symbol);
                assignment_node->rhs->children.push_back(arg);
            } else {
                assignment_node = arg;
            }

            element_list_t list {};
            auto success = add_assignments_to_scope(
                context,
                assignment_node,
                list,
                type_scope);
            if (success) {
                auto argument = dynamic_cast<compiler::declaration*>(list.front());
                args->add(argument);

                auto new_field = builder.make_field(
                    tuple_type,
                    type_scope,
                    argument,
                    previous_field != nullptr ? previous_field->end_offset() : 0);
                tuple_type->fields().add(new_field);
                argument->identifier()->field(new_field);
                previous_field = new_field;
            }

            ++index;
        }

        if (!tuple_type->initialize(_session))
            return nullptr;

        return builder.make_tuple_literal(
            active_scope,
            tuple_type,
            type_params,
            args);
    }

    bool ast_evaluator::compile_module(
            const syntax::ast_node_t* node,
            compiler::module* module) {
        auto& scope_manager = _session.scope_manager();
        auto& module_stack = scope_manager.module_stack();
        module_stack.push(module);

        auto module_scope = module->scope();
        scope_manager.push_scope(module_scope);
        defer({
            scope_manager.pop_scope();
            module_stack.pop();
        });

        for (auto child : node->children) {
            auto expr = evaluate(child);
            if (expr == nullptr)
                return false;
            add_expression_to_scope(module_scope, expr);
            expr->parent_element(module);
        }

        return true;
    }

    void ast_evaluator::pairs_to_symbols_and_type(
            const syntax::ast_node_t* root,
            symbol_list_and_type_t& result) {
        if (root == nullptr
        ||  root->type != syntax::ast_node_type_t::pair) {
            return;
        }

        auto get_type_ref = [&](const syntax::ast_node_t* node) {
            if (result.type_ref != nullptr)
                return;
            if (node == nullptr
            ||  node->type != syntax::ast_node_type_t::type_declaration) {
                return;
            }
            result.type_ref = dynamic_cast<compiler::type_reference*>(evaluate(node));
        };

        auto current_pair = root;
        while (true) {
            if (current_pair->lhs->type != syntax::ast_node_type_t::pair) {
                if (current_pair->rhs != nullptr) {
                    result.symbols.push_back(evaluate(current_pair->rhs));
                    get_type_ref(current_pair->rhs->rhs);
                }
                result.symbols.push_back(evaluate(current_pair->lhs));
                get_type_ref(current_pair->lhs->rhs);
                break;
            }
            result.symbols.push_back(evaluate(current_pair->rhs));
            get_type_ref(current_pair->rhs->rhs);
            current_pair = current_pair->lhs;
        }

        std::reverse(std::begin(result.symbols), std::end(result.symbols));
    }

}