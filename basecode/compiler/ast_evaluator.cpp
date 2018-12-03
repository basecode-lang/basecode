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
#include "ast_evaluator.h"
#include "element_builder.h"

namespace basecode::compiler {

    std::unordered_map<syntax::ast_node_types_t, node_evaluator_callable> ast_evaluator::s_node_evaluators = {
        {syntax::ast_node_types_t::pair,                    std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::label,                   std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::module,                  std::bind(&ast_evaluator::module, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::symbol,                  std::bind(&ast_evaluator::symbol, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::type_list,               std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::proc_call,               std::bind(&ast_evaluator::proc_call, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::statement,               std::bind(&ast_evaluator::statement, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::attribute,               std::bind(&ast_evaluator::attribute, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::directive,               std::bind(&ast_evaluator::directive, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::raw_block,               std::bind(&ast_evaluator::raw_block, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::assignment,              std::bind(&ast_evaluator::assignment, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::expression,              std::bind(&ast_evaluator::expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::nil_literal,             std::bind(&ast_evaluator::nil, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::basic_block,             std::bind(&ast_evaluator::basic_block, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::symbol_part,             std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::line_comment,            std::bind(&ast_evaluator::line_comment, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::block_comment,           std::bind(&ast_evaluator::block_comment, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::argument_list,           std::bind(&ast_evaluator::argument_list, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::if_expression,           std::bind(&ast_evaluator::if_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::parameter_list,          std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::number_literal,          std::bind(&ast_evaluator::number_literal, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::string_literal,          std::bind(&ast_evaluator::string_literal, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::unary_operator,          std::bind(&ast_evaluator::unary_operator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::case_expression,         std::bind(&ast_evaluator::case_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::spread_operator,         std::bind(&ast_evaluator::spread_operator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::from_expression,         std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::proc_expression,         std::bind(&ast_evaluator::proc_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::enum_expression,         std::bind(&ast_evaluator::enum_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::binary_operator,         std::bind(&ast_evaluator::binary_operator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::boolean_literal,         std::bind(&ast_evaluator::boolean_literal, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::else_expression,         std::bind(&ast_evaluator::else_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::break_statement,         std::bind(&ast_evaluator::break_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::with_expression,         std::bind(&ast_evaluator::with_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::while_statement,         std::bind(&ast_evaluator::while_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::type_declaration,        std::bind(&ast_evaluator::type_declaration, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::defer_expression,        std::bind(&ast_evaluator::defer_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::union_expression,        std::bind(&ast_evaluator::union_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::return_statement,        std::bind(&ast_evaluator::return_statement, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::symbol_reference,        std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::for_in_statement,        std::bind(&ast_evaluator::for_in_statement, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::switch_expression,       std::bind(&ast_evaluator::switch_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::lambda_expression,       std::bind(&ast_evaluator::lambda_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::switch_expression,       std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::import_expression,       std::bind(&ast_evaluator::import_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::struct_expression,       std::bind(&ast_evaluator::struct_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::character_literal,       std::bind(&ast_evaluator::character_literal, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::module_expression,       std::bind(&ast_evaluator::module_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::elseif_expression,       std::bind(&ast_evaluator::if_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::subscript_operator,      std::bind(&ast_evaluator::subscript_operator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::with_member_access,      std::bind(&ast_evaluator::with_member_access, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::continue_statement,      std::bind(&ast_evaluator::continue_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::type_tagged_symbol,      std::bind(&ast_evaluator::symbol, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::constant_assignment,     std::bind(&ast_evaluator::assignment, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::namespace_expression,    std::bind(&ast_evaluator::namespace_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::return_argument_list,    std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::array_subscript_list,    std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::fallthrough_statement,   std::bind(&ast_evaluator::fallthrough_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::assignment_source_list,  std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::assignment_target_list,  std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
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
        if (node == nullptr)
            return nullptr;

        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        evaluator_context_t context;
        context.node = node;
        context.scope = scope_manager.current_scope();

        for (const auto& attribute : node->attributes) {
            context.attributes.add(builder.make_attribute(
                scope_manager.current_scope(),
                attribute->token.value,
                evaluate(attribute->lhs.get())));
        }

        for (const auto& comment : node->comments) {
            switch (comment->type) {
                case syntax::ast_node_types_t::line_comment: {
                    context.comments.emplace_back(builder.make_comment(
                        scope_manager.current_scope(),
                        comment_type_t::line,
                        comment->token.value));
                    break;
                }
                case syntax::ast_node_types_t::block_comment: {
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
                return result.element;
            }
        }

        _session.error(
            "P071",
            fmt::format(
                "ast node evaluation failed: id = {}, type = {}",
                node->id,
                syntax::ast_node_type_name(node->type)),
            node->location);

        return nullptr;
    }

    compiler::element* ast_evaluator::evaluate_in_scope(
            const evaluator_context_t& context,
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

    void ast_evaluator::add_procedure_instance(
            const evaluator_context_t& context,
            compiler::procedure_type* proc_type,
            const syntax::ast_node_t* node) {
        auto& builder = _session.builder();

        if (node->children.empty())
            return;

        for (auto& attr : node->attributes) {
            auto attribute = builder.make_attribute(
                proc_type->scope(),
                attr->token.value,
                evaluate(attr->lhs.get()));
            attribute->parent_element(proc_type);
            proc_type->attributes().add(attribute);
        }

        for (const auto& child_node : node->children) {
            switch (child_node->type) {
                case syntax::ast_node_types_t::basic_block: {
                    auto basic_block = dynamic_cast<compiler::block*>(evaluate_in_scope(
                        context,
                        child_node.get(),
                        proc_type->scope()));
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

    compiler::element* ast_evaluator::resolve_symbol_or_evaluate(
            const evaluator_context_t& context,
            const syntax::ast_node_t* node,
            compiler::block* scope,
            bool flag_as_unresolved) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        compiler::element* element = nullptr;
        if (node != nullptr
        &&  node->type == syntax::ast_node_types_t::symbol) {
            qualified_symbol_t qualified_symbol {};
            builder.make_qualified_symbol(qualified_symbol, node);
            element = builder.make_identifier_reference(
                scope_manager.current_scope(),
                qualified_symbol,
                scope_manager.find_identifier(qualified_symbol, scope),
                flag_as_unresolved);
        } else {
            if (scope != nullptr)
                element = evaluate_in_scope(context, node, scope);
            else
                element = evaluate(node);
        }

        return element;
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
        for (size_t i = 0; i < namespaces.size(); i++) {
            if (!namespace_name.empty())
                temp_list.push_back(namespace_name);
            namespace_name = namespaces[i];
            auto var = scope->identifiers().find(namespace_name);
            if (var == nullptr) {
                auto new_scope = builder.make_block(scope, element_type_t::block);
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
                auto expr = var->initializer()->expression();
                if (expr->element_type() == element_type_t::namespace_e) {
                    auto ns = dynamic_cast<namespace_element*>(expr);
                    scope = dynamic_cast<compiler::block*>(ns->expression());
                } else {
                    _session.error(
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

        for (const auto& type_parameter_node : type_parameters_node->children) {
            compiler::type* generic_type = open_generic_type;
            auto param_symbol = builder.make_symbol_from_node(
                type_parameter_node->rhs.get(),
                scope);

            if (type_parameter_node->lhs != nullptr) {
                compiler::type_reference_list_t constraints {};
                for (const auto& symbol : type_parameter_node->lhs->rhs->children) {
                    qualified_symbol_t qualified_symbol {};
                    builder.make_qualified_symbol(qualified_symbol, symbol.get());

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

            auto param_type_ref = builder.make_type_reference(
                scope,
                param_symbol->name(),
                generic_type);
            auto decl = add_identifier_to_scope(
                context,
                param_symbol,
                param_type_ref,
                nullptr,
                0,
                scope);
            decl->identifier()->symbol()->constant(true);
            type_parameters.add(param_symbol, generic_type);
        }
    }

    void ast_evaluator::add_composite_type_fields(
            const evaluator_context_t& context,
            compiler::composite_type* type,
            const syntax::ast_node_t* block) {
        auto& builder = _session.builder();
        compiler::field* previous_field = nullptr;

        for (const auto& child : block->children) {
            if (child->type != syntax::ast_node_types_t::statement) {
                break;
            }

            auto expr_node = child->rhs;
            switch (expr_node->type) {
                case syntax::ast_node_types_t::assignment:
                case syntax::ast_node_types_t::constant_assignment: {
                    element_list_t list {};
                    auto success = add_assignments_to_scope(
                        context,
                        expr_node.get(),
                        list,
                        type->scope());
                    if (success) {
                        auto new_field = builder.make_field(
                            type,
                            type->scope(),
                            dynamic_cast<compiler::declaration*>(list.front()),
                            previous_field != nullptr ? previous_field->end_offset() : 0);
                        type->fields().add(new_field);
                        previous_field = new_field;
                    }
                    break;
                }
                case syntax::ast_node_types_t::symbol: {
                    auto field_decl = declare_identifier(
                        context,
                        expr_node.get(),
                        type->scope());
                    if (field_decl != nullptr) {
                        auto new_field = builder.make_field(
                            type,
                            type->scope(),
                            field_decl,
                            previous_field != nullptr ? previous_field->end_offset() : 0);
                        type->fields().add(new_field);
                        previous_field = new_field;
                    }
                    break;
                }
                default:
                    break;
            }
        }
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

        auto scope = symbol->is_qualified()
                     ? scope_manager.current_top_level()
                     : parent_scope != nullptr ? parent_scope : scope_manager.current_scope();

        scope = add_namespaces_to_scope(context, node, symbol, scope);

        syntax::ast_node_shared_ptr source_node = nullptr;
        if (node != nullptr) {
            source_node = node->rhs->children[source_index];
        }

        auto init_expr = (compiler::element*) nullptr;
        auto init = (compiler::initializer*) nullptr;
        if (node != nullptr) {
            init_expr = evaluate_in_scope(context, source_node.get(), scope);
            if (init_expr != nullptr) {
                if (init_expr->element_type() == element_type_t::symbol) {
                    auto init_symbol = dynamic_cast<compiler::symbol_element*>(init_expr);
                    init_expr = builder.make_identifier_reference(
                        scope,
                        init_symbol->qualified_symbol(),
                        scope_manager.find_identifier(init_symbol->qualified_symbol(), scope));
                }
                if (init_expr->is_constant()) {
                    init = builder.make_initializer(scope, init_expr);
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
                    "P029",
                    "constant assignment (::) is required for module references.",
                    node->location);
                return nullptr;
            }

            auto is_ns = init_expr != nullptr
                         && init_expr->element_type() == element_type_t::namespace_e;
            if (is_ns) {
                _session.error(
                    "P029",
                    "constant assignment (::) is required for namespaces.",
                    node->location);
                return nullptr;
            }

            auto is_type = init_expr != nullptr && init_expr->is_type();
            auto is_type_directive = init_expr != nullptr
                                     && init_expr->element_type() == element_type_t::directive
                                     && dynamic_cast<compiler::directive*>(init_expr)->name() == "type";
            if (is_type || is_type_directive) {
                _session.error(
                    "P029",
                    "constant assignment (::) is required for types.",
                    node->location);
                return nullptr;
            }
        }

        auto new_identifier = builder.make_identifier(scope, symbol, init);
        if (init_expr != nullptr) {
            if (init == nullptr)
                init_expr->parent_element(new_identifier);
        }

        if (init_expr != nullptr && type_ref == nullptr) {
            infer_type_result_t infer_type_result {};
            if (!init_expr->infer_type(_session, infer_type_result)) {
                _session.error(
                    "P019",
                    fmt::format("unable to infer type: {}", new_identifier->symbol()->name()),
                    new_identifier->symbol()->location());
                return nullptr;
            }

            if (infer_type_result.reference == nullptr) {
                infer_type_result.reference = builder.make_type_reference(
                    scope,
                    infer_type_result.inferred_type->symbol()->qualified_symbol(),
                    infer_type_result.inferred_type);
            }

            new_identifier->type_ref(infer_type_result.reference);
            new_identifier->inferred_type(infer_type_result.inferred_type != nullptr);
        } else {
            if (type_ref == nullptr) {
                _session.error(
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
            add_procedure_instance(
                context,
                dynamic_cast<procedure_type*>(init->expression()),
                source_node.get());
        }

        compiler::binary_operator* assign_bin_op = nullptr;

        if (init == nullptr
        &&  init_expr == nullptr
        &&  new_identifier->type_ref() == nullptr) {
            _session.error(
                "P019",
                fmt::format("unable to infer type: {}", new_identifier->symbol()->name()),
                new_identifier->symbol()->location());
            return nullptr;
        } else {
            if (init == nullptr && init_expr != nullptr) {
                // XXX: revisit after type-widening in binary/unary operators is fixed
                //if (symbol->is_constant()) {
                //    _session.error(
                //        "P028",
                //        "constant variables require constant expressions.",
                //        symbol->location());
                //    return nullptr;
                //}
                assign_bin_op = builder.make_binary_operator(
                    scope,
                    operator_type_t::assignment,
                    new_identifier,
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
            evaluate(context.node->lhs.get()));
        result.element->location(context.node->location);
        return true;
    }

    bool ast_evaluator::directive(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        auto expression = evaluate(context.node->lhs.get());
        if (expression == nullptr)
            return false;

        auto directive_element = builder.make_directive(
            scope_manager.current_scope(),
            context.node->token.value,
            expression);
        directive_element->location(context.node->location);
        directive_element->evaluate(_session);
        result.element = directive_element;

        return true;
    }

    bool ast_evaluator::module(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& program = _session.program();
        auto& scope_manager = _session.scope_manager();

        auto module_block = builder.make_block(program.block(), element_type_t::module_block);
        auto module = builder.make_module(program.block(), module_block);
        module->source_file(_session.current_source_file());
        program.block()->blocks().push_back(module_block);

        scope_manager.push_scope(module_block);
        scope_manager.top_level_stack().push(module_block);
        scope_manager.module_stack().push(module);

        for (auto it = context.node->children.begin();
                 it != context.node->children.end();
                 ++it) {
            auto expr = evaluate((*it).get());
            if (expr == nullptr)
                return false;
            add_expression_to_scope(module_block, expr);
            expr->parent_element(module);
        }

        scope_manager.top_level_stack().pop();
        scope_manager.module_stack().pop();

        result.element = module;

        return true;
    }

    bool ast_evaluator::break_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        compiler::label* label = nullptr;
        if (context.node->lhs != nullptr) {
            label = builder.make_label(
                scope_manager.current_scope(),
                context.node->lhs->token.value);
        }

        result.element = builder.make_break(scope_manager.current_scope(), label);
        return true;
    }

    bool ast_evaluator::continue_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        compiler::label* label = nullptr;
        if (context.node->lhs != nullptr) {
            label = builder.make_label(
                scope_manager.current_scope(),
                context.node->lhs->token.value);
        }

        result.element = builder.make_continue(scope_manager.current_scope(), label);
        return true;
    }

    bool ast_evaluator::module_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto expr = resolve_symbol_or_evaluate(context, context.node->rhs.get());
        auto reference = _session.builder().make_module_reference(
            _session.scope_manager().current_scope(),
            expr);

        std::string path;
        if (expr->is_constant() && expr->as_string(path)) {
            boost::filesystem::path source_path(path);
            auto current_source_file = _session.current_source_file();
            if (current_source_file != nullptr
            &&  source_path.is_relative()) {
                source_path = boost::filesystem::absolute(
                    source_path,
                    current_source_file->path().parent_path());
            }
            auto source_file = _session.add_source_file(source_path);
            auto module = _session.compile_module(source_file);
            if (module == nullptr) {
                _session.error(
                    "C021",
                    "unable to load module.",
                    context.node->rhs->location);
                return false;
            }
            reference->module(module);
            result.element = reference;
        } else {
            _session.error(
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

        std::vector<uint8_t> _buffer;
        for (auto c : context.node->token.value)
            _buffer.push_back(c);

        auto ch = _buffer[0];
        rune_t rune = ch;
        if (ch >= 0x80) {
            auto cp = common::utf8_decode(
                (char*)(_buffer.data()),
                _buffer.size());
            rune = cp.value;
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
            evaluate(context.node->rhs.get()));
        return true;
    }

    bool ast_evaluator::expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = _session.builder().make_expression(
            _session.scope_manager().current_scope(),
            evaluate(context.node->lhs.get()));
        result.element->location(context.node->location);
        return true;
    }

    bool ast_evaluator::argument_list(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        auto args = builder.make_argument_list(scope_manager.current_scope());
        for (const auto& arg_node : context.node->children) {
            compiler::element* arg = nullptr;

            switch (arg_node->type) {
                case syntax::ast_node_types_t::assignment: {
                    auto lhs = evaluate(arg_node->lhs->children.front().get());
                    auto rhs = resolve_symbol_or_evaluate(
                        context,
                        arg_node->rhs->children.front().get());
                    if (lhs != nullptr && rhs != nullptr)
                        arg = builder.make_argument_pair(
                            scope_manager.current_scope(),
                            lhs,
                            rhs);
                    break;
                }
                default: {
                    arg = resolve_symbol_or_evaluate(context, arg_node.get());
                    break;
                }
            }

            if (arg == nullptr) {
                // XXX: error
                return false;
            }

            args->add(arg);
            arg->parent_element(args);
        }

        // XXX: the order of args is not correct.  maybe pairs in parser is reordering items.
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
            resolve_symbol_or_evaluate(context, context.node->rhs.get()));
        return true;
    }

    bool ast_evaluator::spread_operator(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        compiler::element* expr = nullptr;
        if (context.node->lhs != nullptr) {
            expr = resolve_symbol_or_evaluate(context, context.node->lhs.get());
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
            lhs = convert_predicate(context, context.node->lhs.get(), scope);
            rhs = convert_predicate(context, context.node->rhs.get(), scope);
        } else {
            compiler::block* type_scope = nullptr;

            lhs = resolve_symbol_or_evaluate(
                context,
                context.node->lhs.get());
            infer_type_result_t infer_type_result {};
            if (lhs == nullptr
            || !lhs->infer_type(_session, infer_type_result)) {
                _session.error(
                    "P052",
                    "unable to infer type.",
                    context.node->lhs->location);
                return false;
            }

            // XXX: there's a bug here and it goes like this...
            //
            // member access is expecting a resolved type here; however, it may be an unknown_type and
            // that is ok.  however, that means the field resolution process here is going to fail
            // because the lhs type is unknown at this point.  this code needs to be reworked so
            // we can defer this processing until after all unknown_type instances have been resolved.
            //
            if (infer_type_result.inferred_type->is_composite_type()) {
                compiler::composite_type* composite_type = nullptr;
                if (infer_type_result.inferred_type->is_pointer_type()) {
                    auto pointer_type = dynamic_cast<compiler::pointer_type*>(infer_type_result.inferred_type);
                    composite_type = dynamic_cast<compiler::composite_type*>(pointer_type->base_type_ref()->type());
                    if (is_member_access) {
                        lhs = builder.make_unary_operator(
                            scope,
                            operator_type_t::pointer_dereference,
                            lhs);
                    }
                } else {
                    composite_type = dynamic_cast<compiler::composite_type*>(infer_type_result.inferred_type);
                }
                type_scope = composite_type->scope();
            } else {
                if (is_member_access) {
                    _session.error(
                        lhs,
                        "P053",
                        "member access requires lhs composite type.",
                        lhs->location());
                    return false;
                }
            }

            rhs = resolve_symbol_or_evaluate(
                context,
                context.node->rhs.get(),
                type_scope);
            if (is_member_access) {
                infer_type_result_t rhs_type_result {};
                if (!rhs->infer_type(_session, rhs_type_result)) {
                    _session.error(
                        rhs,
                        "P052",
                        "unable to infer type.",
                        rhs->location());
                    return false;
                }

                if (rhs_type_result.inferred_type->is_pointer_type()) {
                    auto pointer_type = dynamic_cast<compiler::pointer_type*>(rhs_type_result.inferred_type);
                    if (pointer_type->base_type_ref()->is_composite_type()) {
                        rhs = builder.make_unary_operator(
                            scope,
                            operator_type_t::pointer_dereference,
                            rhs);
                    }
                }
            }
        }

        result.element = builder.make_binary_operator(
            scope,
            it->second,
            lhs,
            rhs);

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
            context.node->lhs.get(),
            scope);
        auto body = evaluate(context.node->rhs.get());

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
        for (const auto& arg_node : context.node->rhs->children) {
            auto arg = resolve_symbol_or_evaluate(context, arg_node.get());
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
        builder.make_qualified_symbol(qualified_symbol, context.node->lhs.get());

        compiler::identifier_reference* from_reference = nullptr;
        if (context.node->rhs != nullptr) {
            from_reference = dynamic_cast<compiler::identifier_reference*>(
                resolve_symbol_or_evaluate(context, context.node->rhs.get()));
            qualified_symbol.namespaces.insert(
                qualified_symbol.namespaces.begin(),
                from_reference->symbol().name);
        }

        auto identifier_reference = builder.make_identifier_reference(
            scope_manager.current_scope(),
            qualified_symbol,
            scope_manager.find_identifier(qualified_symbol));
        auto import = builder.make_import(
            scope_manager.current_scope(),
            identifier_reference,
            from_reference,
            dynamic_cast<compiler::module*>(scope_manager.current_top_level()->parent_element()));
        add_expression_to_scope(scope_manager.current_scope(), import);

        result.element = import;

        return true;
    }

    bool ast_evaluator::subscript_operator(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = _session.builder().make_binary_operator(
            _session.scope_manager().current_scope(),
            operator_type_t::subscript,
            resolve_symbol_or_evaluate(context, context.node->lhs.get()),
            evaluate(context.node->rhs.get()));
        return true;
    }

    bool ast_evaluator::basic_block(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& scope_manager = _session.scope_manager();
        auto active_scope = scope_manager.push_new_block();

        for (auto it = context.node->children.begin();
                 it != context.node->children.end();
                 ++it) {
            auto current_node = *it;
            auto expr = evaluate(current_node.get());
            if (expr == nullptr) {
                _session.error(
                    "C024",
                    "invalid statement",
                    current_node->location);
                return false;
            }
            add_expression_to_scope(active_scope, expr);
            expr->parent_element(active_scope);
        }

        result.element = scope_manager.pop_scope();
        return true;
    }

    bool ast_evaluator::proc_call(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        compiler::argument_list* args = nullptr;
        auto argument_list = evaluate(context.node->rhs.get());
        if (argument_list == nullptr)
            return false;
        args = dynamic_cast<compiler::argument_list*>(argument_list);

        auto symbol_node = context.node->lhs->rhs.get();

        qualified_symbol_t proc_name {};
        builder.make_qualified_symbol(proc_name, symbol_node);

        auto type_params = builder.make_tagged_type_list_from_node(symbol_node);

        using element_maker_t = std::function<bool (
            compiler::element_builder&,
            compiler::scope_manager&,
            evaluator_result_t&,
            const type_reference_list_t&,
            compiler::argument_list*)>;
        static std::unordered_map<std::string, element_maker_t> s_makers {
            {
                "new",
                [](compiler::element_builder& builder,
                        compiler::scope_manager& scope_manager,
                        evaluator_result_t& result,
                        const type_reference_list_t& type_params,
                        compiler::argument_list* args) {
                    result.element = builder.make_user_literal(
                        scope_manager.current_scope(),
                        type_params[0],
                        args);
                    return true;
                }
            },
            {
                "map",
                [](compiler::element_builder& builder,
                       compiler::scope_manager& scope_manager,
                       evaluator_result_t& result,
                       const type_reference_list_t& type_params,
                       compiler::argument_list* args) {
                    auto map_type = builder.make_map_type(
                        scope_manager.current_scope(),
                        type_params[0],
                        type_params[1]);
                    result.element = builder.make_map_literal(
                        scope_manager.current_scope(),
                        map_type,
                        args);
                    return true;
                }
            },
            {
                "cast",
                [](compiler::element_builder& builder,
                       compiler::scope_manager& scope_manager,
                       evaluator_result_t& result,
                       const type_reference_list_t& type_params,
                       compiler::argument_list* args) {
                    // XXX: should cast accept an argument_list?
                    auto cast_element = builder.make_cast(
                        scope_manager.current_scope(),
                        type_params[0],
                        args->elements().front());
//                    cast_element->location(context.node->location);
//                    cast_element->type_location(context.node->lhs->lhs->location);
                    result.element = cast_element;
                    return true;
                }
            },
            {
                "array",
                [](compiler::element_builder& builder,
                       compiler::scope_manager& scope_manager,
                       evaluator_result_t& result,
                       const type_reference_list_t& type_params,
                       compiler::argument_list* args) {
                    result.element = builder.make_array_literal(
                        scope_manager.current_scope(),
                        type_params[0],
                        args);
                    return true;
                }
            },
            {
                "transmute",
                [](compiler::element_builder& builder,
                       compiler::scope_manager& scope_manager,
                       evaluator_result_t& result,
                       const type_reference_list_t& type_params,
                       compiler::argument_list* args) {
                    // XXX: like cast, should transmute support argument_list directly?
                    auto transmute_element = builder.make_transmute(
                        scope_manager.current_scope(),
                        type_params[0],
                        args->elements().front());
//                    transmute_element->location(context.node->location);
//                    transmute_element->type_location(context.node->lhs->lhs->location);
                    result.element = transmute_element;
                    return true;
                }
            },
        };

        auto maker_it = s_makers.find(proc_name.name);
        if (maker_it != s_makers.end()) {
            return maker_it->second(builder, scope_manager, result, type_params, args);
        } else if (proc_name.name == "tuple") {
            result.element = make_tuple_literal(context, scope_manager.current_scope());
        } else {
            auto intrinsic = compiler::intrinsic::intrinsic_for_call(
                _session,
                scope_manager.current_scope(),
                args,
                proc_name,
                type_params);
            if (intrinsic != nullptr) {
                result.element = intrinsic;
                return true;
            }

            auto proc_identifier = scope_manager.find_identifier(proc_name);
            result.element = builder.make_procedure_call(
                scope_manager.current_scope(),
                builder.make_identifier_reference(
                    scope_manager.current_scope(),
                    proc_name,
                    proc_identifier),
                args,
                type_params);
            result.element->location(context.node->location);
        }

        return true;
    }

    bool ast_evaluator::statement(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        label_list_t labels {};

        for (const auto& label : context.node->labels) {
            labels.push_back(builder.make_label(
                scope_manager.current_scope(),
                label->token.value));
        }

        compiler::element* expr = nullptr;
        if (context.node->rhs != nullptr) {
            expr = evaluate(context.node->rhs.get());
            if (expr == nullptr)
                return false;

            if (expr->element_type() == element_type_t::symbol) {
                auto type_ref = dynamic_cast<compiler::type_reference*>(evaluate(context.node->rhs->rhs.get()));
                if (type_ref == nullptr)
                    return false;
                expr = add_identifier_to_scope(
                    context,
                    dynamic_cast<compiler::symbol_element*>(expr),
                    type_ref,
                    nullptr,
                    0);
            }
        }

        result.element = builder.make_statement(
            scope_manager.current_scope(),
            labels,
            expr);

        return true;
    }

    bool ast_evaluator::enum_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        auto active_scope = scope_manager.current_scope();
        auto enum_scope = builder.make_block(active_scope, element_type_t::block);
        auto enum_type = builder.make_enum_type(active_scope, enum_scope);
        active_scope->types().add(enum_type);

        add_type_parameters(
            context,
            enum_scope,
            context.node->lhs.get(),
            enum_type->type_parameters());

        add_composite_type_fields(
            context,
            enum_type,
            context.node->rhs.get());

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
            expr = evaluate(context.node->lhs.get());
        auto scope = dynamic_cast<compiler::block*>(evaluate(context.node->rhs.get()));

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
            resolve_symbol_or_evaluate(
                context,
                context.node->lhs.get()),
            dynamic_cast<compiler::block*>(evaluate(context.node->rhs.get())));
        return true;
    }

    bool ast_evaluator::switch_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        auto expr = resolve_symbol_or_evaluate(context, context.node->lhs.get());
        auto scope = dynamic_cast<compiler::block*>(evaluate(context.node->rhs.get()));

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
            evaluate(context.node->lhs.get()));
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
        auto struct_scope = builder.make_block(
            active_scope,
            element_type_t::block);

        auto struct_type = builder.make_struct_type(
            active_scope,
            struct_scope);
        active_scope->types().add(struct_type);

        add_type_parameters(
            context,
            struct_scope,
            context.node->lhs.get(),
            struct_type->type_parameters());

        add_composite_type_fields(
            context,
            struct_type,
            context.node->rhs.get());

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
        auto union_scope = builder.make_block(active_scope, element_type_t::block);
        auto union_type = builder.make_union_type(active_scope, union_scope);
        active_scope->types().add(union_type);

        add_type_parameters(
            context,
            union_scope,
            context.node->lhs.get(),
            union_type->type_parameters());

        add_composite_type_fields(
            context,
            union_type,
            context.node->rhs.get());

        if (!union_type->initialize(_session))
            return false;

        result.element = union_type;
        return true;
    }

    bool ast_evaluator::else_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = evaluate(context.node->children[0].get());
        return true;
    }

    bool ast_evaluator::if_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto scope = _session.scope_manager().current_scope();

        auto predicate = convert_predicate(context, context.node->lhs.get(), scope);
        auto true_branch = evaluate(context.node->children[0].get());
        auto false_branch = evaluate(context.node->rhs.get());
        result.element = _session.builder().make_if(
            scope,
            predicate,
            true_branch,
            false_branch,
            context.node->type == syntax::ast_node_types_t::elseif_expression);
        return true;
    }

    bool ast_evaluator::proc_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        auto active_scope = scope_manager.current_scope();
        auto block_scope = builder.make_block(active_scope, element_type_t::block);
        auto proc_type = builder.make_procedure_type(active_scope, block_scope);
        active_scope->types().add(proc_type);

        add_type_parameters(
            context,
            block_scope,
            context.node->lhs->lhs.get(),
            proc_type->type_parameters());

        compiler::field* return_field = nullptr;
        auto return_type_node = context.node->lhs->rhs;
        if (return_type_node != nullptr) {
            auto return_identifier = builder.make_identifier(
                block_scope,
                builder.make_symbol(block_scope, "_retval"),
                nullptr);
            return_identifier->usage(identifier_usage_t::stack);

            auto type_ref = dynamic_cast<compiler::type_reference*>(evaluate_in_scope(
                context,
                return_type_node.get(),
                block_scope));
            if (type_ref->is_unknown_type()) {
                _session.scope_manager()
                    .identifiers_with_unknown_types()
                    .push_back(return_identifier);
            }
            return_identifier->type_ref(type_ref);
            return_field = builder.make_field(
                proc_type,
                block_scope,
                builder.make_declaration(block_scope, return_identifier, nullptr),
                0);
            proc_type->return_type(return_field);
        }

        compiler::field* param_field = nullptr;
        auto& parameter_map = proc_type->parameters();

        for (const auto& param_node : context.node->rhs->children) {
            switch (param_node->type) {
                case syntax::ast_node_types_t::assignment: {
                    element_list_t list {};
                    auto success = add_assignments_to_scope(
                        context,
                        param_node.get(),
                        list,
                        block_scope);
                    if (success) {
                        auto param_decl = dynamic_cast<compiler::declaration*>(list.front());
                        param_decl->identifier()->usage(identifier_usage_t::stack);
                        param_field = builder.make_field(
                            proc_type,
                            block_scope,
                            param_decl,
                            param_field != nullptr ? param_field->end_offset() : 0);
                        parameter_map.add(param_field);
                    } else {
                        return false;
                    }
                    break;
                }
                case syntax::ast_node_types_t::symbol: {
                    auto param_decl = declare_identifier(
                        context,
                        param_node.get(),
                        block_scope);
                    if (param_decl != nullptr) {
                        param_decl->identifier()->usage(identifier_usage_t::stack);
                        param_field = builder.make_field(
                            proc_type,
                            block_scope,
                            param_decl,
                            param_field != nullptr ? param_field->end_offset() : 0);
                        parameter_map.add(param_field);
                    } else {
                        return false;
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }

        result.element = proc_type;

        return true;
    }

    bool ast_evaluator::lambda_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();
        auto open_generic_type = scope_manager.find_generic_type({});

        auto active_scope = scope_manager.current_scope();
        auto block_scope = builder.make_block(active_scope, element_type_t::block);
        auto proc_type = builder.make_procedure_type(active_scope, block_scope);
        active_scope->types().add(proc_type);

        add_type_parameters(
            context,
            block_scope,
            context.node->lhs->lhs.get(),
            proc_type->type_parameters());

        compiler::field* return_field = nullptr;
        auto return_type_node = context.node->lhs->rhs;
        if (return_type_node != nullptr) {
            auto return_identifier = builder.make_identifier(
                block_scope,
                builder.make_symbol(block_scope, "_retval"),
                nullptr);
            return_identifier->usage(identifier_usage_t::stack);

            auto type_ref = dynamic_cast<compiler::type_reference*>(evaluate_in_scope(
                context,
                return_type_node.get(),
                block_scope));
            if (type_ref->is_unknown_type()) {
                _session.scope_manager()
                    .identifiers_with_unknown_types()
                    .push_back(return_identifier);
            }
            return_identifier->type_ref(type_ref);
            return_field = builder.make_field(
                proc_type,
                block_scope,
                builder.make_declaration(block_scope, return_identifier, nullptr),
                0);
            proc_type->return_type(return_field);
        }

        compiler::field* param_field = nullptr;
        auto& parameter_map = proc_type->parameters();

        for (const auto& param_node : context.node->rhs->children) {
            switch (param_node->type) {
                case syntax::ast_node_types_t::symbol: {
                    context.decl_type_ref = builder.make_type_reference(
                        block_scope,
                        open_generic_type->name(),
                        open_generic_type);
                    auto param_decl = declare_identifier(
                        context,
                        param_node.get(),
                        block_scope);
                    if (param_decl != nullptr) {
                        param_decl->identifier()->usage(identifier_usage_t::stack);
                        param_field = builder.make_field(
                            proc_type,
                            block_scope,
                            param_decl,
                            param_field != nullptr ? param_field->end_offset() : 0);
                        parameter_map.add(param_field);
                    } else {
                        return false;
                    }
                    break;
                }
                default: {
                    break;
                }
            }
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

        std::stack<syntax::ast_node_shared_ptr> type_nodes {};
        auto current = context.node->lhs;
        while (true) {
            type_nodes.push(current);
            if (current->rhs == nullptr)
                break;
            current = current->rhs;
        }

        compiler::element_list_t subscripts {};
        while (!type_nodes.empty()) {
            current = type_nodes.top();
            type_nodes.pop();

            switch (current->type) {
                case syntax::ast_node_types_t::symbol: {
                    qualified_symbol_t type_name{};
                    builder.make_qualified_symbol(
                        type_name,
                        current.get());
                    auto symbol = builder.make_symbol_from_node(current.get(), scope);
                    auto type = scope_manager.find_type(type_name, scope);
                    if (type == nullptr) {
                        type = builder.make_unknown_type(
                            scope,
                            symbol);
                    }
                    type_decl_ref = builder.make_type_reference(
                        scope,
                        type->symbol()->qualified_symbol(),
                        type);
                    break;
                }
                case syntax::ast_node_types_t::pointer_declaration: {
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
                        qualified_symbol_t {},
                        type);
                    break;
                }
                case syntax::ast_node_types_t::subscript_declaration: {
                    auto is_dynamic = current->lhs == nullptr;
                    if (!is_dynamic) {
                        subscripts.push_back(resolve_symbol_or_evaluate(
                            context,
                            current->lhs.get()));
                    }

                    while (!type_nodes.empty()) {
                        auto subscript_node = type_nodes.top();
                        if (subscript_node->type != syntax::ast_node_types_t::subscript_declaration)
                            break;

                        if (is_dynamic) {
                            // XXX: error
                            return false;
                        }

                        subscripts.push_back(resolve_symbol_or_evaluate(
                            context,
                            subscript_node->lhs.get()));

                        type_nodes.pop();
                    }

                    auto type = scope_manager.find_array_type(
                        type_decl_ref->type(),
                        subscripts,
                        scope);
                    if (type == nullptr) {
                        type = builder.make_array_type(
                            scope,
                            builder.make_block(scope, element_type_t::block),
                            type_decl_ref,
                            subscripts);
                    }
                    type_decl_ref = builder.make_type_reference(
                        scope,
                        qualified_symbol_t {},
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

        auto rhs = resolve_symbol_or_evaluate(
            context,
            context.node->rhs.get());
        if (rhs == nullptr) {
            // XXX: error
            return false;
        }

        auto lhs = evaluate(context.node->lhs.get());
        if (lhs == nullptr || lhs->element_type() != element_type_t::symbol) {
            // XXX: error
            return false;
        }

        compiler::type_reference* type_ref = nullptr;
        if (context.node->lhs->rhs != nullptr) {
            auto ref = evaluate(context.node->lhs->rhs.get());
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

        auto for_scope = builder.make_block(
            scope_manager.current_scope(),
            element_type_t::block);
        auto induction_decl = add_identifier_to_scope(
            context,
            dynamic_cast<compiler::symbol_element*>(lhs),
            type_ref,
            nullptr,
            0,
            for_scope);

        auto block = evaluate_in_scope(
            context,
            context.node->children.front().get(),
            for_scope);
        auto body = dynamic_cast<compiler::block*>(block);

        result.element = builder.make_for(
            scope_manager.current_scope(),
            induction_decl,
            rhs,
            body);

        return true;
    }

    bool ast_evaluator::with_member_access(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto member_access_bin_op = _session.ast_builder().binary_operator_node(
            context.node->lhs,
            syntax::s_period_literal,
            context.node->rhs);
        context.node = member_access_bin_op.get();
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

        const auto& target_list = node->lhs;
        const auto& source_list = node->rhs;

        const bool is_constant_assignment = node->type == syntax::ast_node_types_t::constant_assignment;

        if (target_list->children.size() != source_list->children.size()) {
            _session.error(
                "P027",
                "the number of left-hand-side targets must match"
                " the number of right-hand-side expressions.",
                source_list->location);
            return false;
        }

        for (size_t i = 0; i < target_list->children.size(); i++) {
            const auto& target_symbol = target_list->children[i];

            auto is_binary_op = true;
            auto target_element = resolve_symbol_or_evaluate(
                context,
                target_symbol.get(),
                scope,
                false);

            if (target_element->element_type() == element_type_t::identifier_reference) {
                auto identifier_ref = dynamic_cast<compiler::identifier_reference*>(target_element);
                if (identifier_ref->resolved()) {
                    if (identifier_ref->identifier()->symbol()->is_constant()) {
                        _session.error(
                            "P028",
                            "constant variables cannot be modified.",
                            target_symbol->location);
                        return false;
                    }
                } else {
                    is_binary_op = false;
                }
            }

            if (is_binary_op) {
                auto rhs = resolve_symbol_or_evaluate(
                    context,
                    source_list->children[i].get(),
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
                    context,
                    target_symbol.get(),
                    scope);

                auto symbol = dynamic_cast<compiler::symbol_element*>(lhs);
                symbol->constant(is_constant_assignment);

                auto type_ref = dynamic_cast<compiler::type_reference*>(evaluate_in_scope(
                    context,
                    target_symbol->rhs.get(),
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

    compiler::declaration* ast_evaluator::declare_identifier(
            const evaluator_context_t& context,
            const syntax::ast_node_t* node,
            compiler::block* scope) {
        auto identifier = dynamic_cast<compiler::symbol_element*>(evaluate_in_scope(
            context,
            node,
            scope));
        auto type_ref = context.decl_type_ref != nullptr ?
            context.decl_type_ref :
            dynamic_cast<compiler::type_reference*>(evaluate_in_scope(
                context,
                node->rhs.get(),
                scope));

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

        auto predicate = resolve_symbol_or_evaluate(context, node, scope);
        if (predicate->element_type() != element_type_t::binary_operator) {
            infer_type_result_t infer_type_result {};
            if (!predicate->infer_type(_session, infer_type_result))
                return nullptr;

            if (infer_type_result.inferred_type->element_type() != element_type_t::bool_type) {
                _session.error(
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

    compiler::type_literal* ast_evaluator::make_tuple_literal(
            const evaluator_context_t& context,
            compiler::block* scope) {
        auto& builder = _session.builder();
        auto& ast_builder = _session.ast_builder();
        auto& scope_manager = _session.scope_manager();

        auto active_scope = scope_manager.current_scope();

        auto type_scope = builder.make_block(active_scope, element_type_t::block);
        auto tuple_type = builder.make_tuple_type(active_scope, type_scope);
        active_scope->types().add(tuple_type);

        // XXX: should mixed named and unnamed fields be allowed?
        auto args = builder.make_argument_list(active_scope);

        size_t index = 0;
        compiler::field* previous_field = nullptr;
        for (const auto& arg : context.node->rhs->children) {
            syntax::ast_node_shared_ptr assignment_node;

            if (arg->type != syntax::ast_node_types_t::assignment) {
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
                assignment_node.get(),
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
                previous_field = new_field;
            }

            ++index;
        }

        if (!tuple_type->initialize(_session))
            return nullptr;

        return builder.make_tuple_literal(
            active_scope,
            tuple_type,
            args);
    }

};