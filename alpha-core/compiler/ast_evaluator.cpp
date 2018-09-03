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
#include <compiler/elements/type.h>
#include <compiler/elements/cast.h>
#include <compiler/elements/label.h>
#include <compiler/elements/import.h>
#include <compiler/elements/module.h>
#include <compiler/elements/comment.h>
#include <compiler/elements/program.h>
#include <compiler/elements/any_type.h>
#include <compiler/elements/intrinsic.h>
#include <compiler/elements/raw_block.h>
#include <compiler/elements/bool_type.h>
#include <compiler/elements/attribute.h>
#include <compiler/elements/directive.h>
#include <compiler/elements/statement.h>
#include <compiler/elements/type_info.h>
#include <compiler/elements/transmute.h>
#include <compiler/elements/expression.h>
#include <compiler/elements/identifier.h>
#include <compiler/elements/if_element.h>
#include <compiler/elements/array_type.h>
#include <compiler/elements/tuple_type.h>
#include <compiler/elements/initializer.h>
#include <compiler/elements/module_type.h>
#include <compiler/elements/string_type.h>
#include <compiler/elements/numeric_type.h>
#include <compiler/elements/unknown_type.h>
#include <compiler/elements/pointer_type.h>
#include <compiler/elements/argument_list.h>
#include <compiler/elements/float_literal.h>
#include <compiler/elements/type_reference.h>
#include <compiler/elements/string_literal.h>
#include <compiler/elements/unary_operator.h>
#include <compiler/elements/composite_type.h>
#include <compiler/elements/procedure_type.h>
#include <compiler/elements/return_element.h>
#include <compiler/elements/procedure_call.h>
#include <compiler/elements/namespace_type.h>
#include <compiler/elements/symbol_element.h>
#include <compiler/elements/boolean_literal.h>
#include <compiler/elements/binary_operator.h>
#include <compiler/elements/integer_literal.h>
#include <compiler/elements/module_reference.h>
#include <compiler/elements/namespace_element.h>
#include <compiler/elements/size_of_intrinsic.h>
#include <compiler/elements/procedure_instance.h>
#include <compiler/elements/identifier_reference.h>
#include "ast_evaluator.h"
#include "element_builder.h"
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
        {syntax::ast_node_types_t::label_list,              std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::basic_block,             std::bind(&ast_evaluator::basic_block, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::symbol_part,             std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::line_comment,            std::bind(&ast_evaluator::line_comment, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::null_literal,            std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::block_comment,           std::bind(&ast_evaluator::block_comment, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::argument_list,           std::bind(&ast_evaluator::argument_list, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::if_expression,           std::bind(&ast_evaluator::if_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::parameter_list,          std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::number_literal,          std::bind(&ast_evaluator::number_literal, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::string_literal,          std::bind(&ast_evaluator::string_literal, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::unary_operator,          std::bind(&ast_evaluator::unary_operator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::cast_expression,         std::bind(&ast_evaluator::cast_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::from_expression,         std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::proc_expression,         std::bind(&ast_evaluator::proc_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::enum_expression,         std::bind(&ast_evaluator::enum_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::binary_operator,         std::bind(&ast_evaluator::binary_operator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::boolean_literal,         std::bind(&ast_evaluator::boolean_literal, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::else_expression,         std::bind(&ast_evaluator::else_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::while_statement,         std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::break_statement,         std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::with_expression,         std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::type_identifier,         std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::defer_expression,        std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::union_expression,        std::bind(&ast_evaluator::union_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::return_statement,        std::bind(&ast_evaluator::return_statement, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::symbol_reference,        std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::for_in_statement,        std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::switch_expression,       std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::import_expression,       std::bind(&ast_evaluator::import_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::struct_expression,       std::bind(&ast_evaluator::struct_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::character_literal,       std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::array_constructor,       std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::module_expression,       std::bind(&ast_evaluator::module_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::elseif_expression,       std::bind(&ast_evaluator::if_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::continue_statement,      std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::constant_assignment,     std::bind(&ast_evaluator::assignment, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::transmute_expression,    std::bind(&ast_evaluator::transmute_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::namespace_expression,    std::bind(&ast_evaluator::namespace_expression, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::subscript_expression,    std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::return_argument_list,    std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::array_subscript_list,    std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::assignment_source_list,  std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {syntax::ast_node_types_t::assignment_target_list,  std::bind(&ast_evaluator::noop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
    };

    ///////////////////////////////////////////////////////////////////////////

    ast_evaluator::ast_evaluator(compiler::session& session) : _session(session) {
    }

    element* ast_evaluator::evaluate(
            const syntax::ast_node_t* node,
            element_type_t default_block_type) {
        if (node == nullptr)
            return nullptr;

        evaluator_context_t context;
        context.node = node;
        context.scope = _session.scope_manager().current_scope();
        context.default_block_type = default_block_type;

        auto it = s_node_evaluators.find(node->type);
        if (it != s_node_evaluators.end()) {
            evaluator_result_t result {};
            if (it->second(this, context, result)) {
                return result.element;
            }
        } else {
            _session.error(
                "P071",
                fmt::format(
                    "ast node evaluation failed: id = {}, type = {}",
                    node->id,
                    syntax::ast_node_type_name(node->type)),
                node->location);
        }

        return nullptr;
    }

    void ast_evaluator::apply_attributes(
            const evaluator_context_t& context,
            compiler::element* element,
            const syntax::ast_node_t* node) {
        if (node == nullptr)
            return;

        for (auto it = node->children.begin();
             it != node->children.end();
             ++it) {
            const auto& child_node = *it;
            if (child_node->type == syntax::ast_node_types_t::attribute) {
                auto attribute = dynamic_cast<compiler::attribute*>(evaluate(child_node.get()));
                attribute->parent_element(element);

                auto& attributes = element->attributes();
                attributes.add(attribute);
            }
        }
    }

    element* ast_evaluator::evaluate_in_scope(
            const evaluator_context_t& context,
            const syntax::ast_node_t* node,
            compiler::block* scope,
            element_type_t default_block_type) {
        auto& scope_manager = _session.scope_manager();

        if (scope != nullptr)
            scope_manager.push_scope(scope);

        auto result = evaluate(
            node,
            default_block_type);

        if (scope != nullptr)
            scope_manager.pop_scope();

        return result;
    }

    void ast_evaluator::add_procedure_instance(
            const evaluator_context_t& context,
            compiler::procedure_type* proc_type,
            const syntax::ast_node_t* node) {
        if (node->children.empty())
            return;

        for (const auto& child_node : node->children) {
            switch (child_node->type) {
                case syntax::ast_node_types_t::attribute: {
                    auto attribute = _session.builder().make_attribute(
                        proc_type->scope(),
                        child_node->token.value,
                        evaluate(child_node->lhs.get()));
                    attribute->parent_element(proc_type);
                    proc_type->attributes().add(attribute);
                    break;
                }
                case syntax::ast_node_types_t::basic_block: {
                    auto basic_block = dynamic_cast<compiler::block*>(evaluate_in_scope(
                        context,
                        child_node.get(),
                        proc_type->scope(),
                        element_type_t::proc_instance_block));
                    auto instance = _session.builder().make_procedure_instance(
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
            case element_type_t::comment: {
                auto comment = dynamic_cast<compiler::comment*>(expr);
                scope->comments().emplace_back(comment);
                break;
            }
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
            const syntax::ast_node_t* node) {
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
                scope_manager.find_identifier(qualified_symbol));
        } else {
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

        auto namespace_type = scope_manager.find_type(qualified_symbol_t {
            .name = "namespace"
        });

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

    void ast_evaluator::add_composite_type_fields(
            const evaluator_context_t& context,
            compiler::composite_type* type,
            const syntax::ast_node_t* block) {
        auto& builder = _session.builder();

        for (const auto& child : block->children) {
            if (child->type != syntax::ast_node_types_t::statement) {
                break;
            }
            auto expr_node = child->rhs;
            switch (expr_node->type) {
                case syntax::ast_node_types_t::assignment: {
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
                            dynamic_cast<compiler::identifier*>(list.front()));
                        type->fields().add(new_field);
                    }
                    break;
                }
                case syntax::ast_node_types_t::symbol: {
                    auto field_identifier = declare_identifier(
                        context,
                        expr_node.get(),
                        type->scope());
                    if (field_identifier != nullptr) {
                        auto new_field = builder.make_field(
                            type,
                            type->scope(),
                            field_identifier);
                        type->fields().add(new_field);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    compiler::identifier* ast_evaluator::add_identifier_to_scope(
            const evaluator_context_t& context,
            compiler::symbol_element* symbol,
            type_find_result_t& type_find_result,
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
                    auto identifier = scope_manager.find_identifier(init_symbol->qualified_symbol());
                    if (identifier != nullptr) {
                        auto type_initializer = identifier->initializer();
                        if (type_initializer != nullptr
                        &&  type_initializer->expression()->element_type() == element_type_t::type_reference) {
                            if (symbol->is_constant()) {
                                init_expr = type_initializer->expression();
                            } else {
                                _session.error(
                                    "P029",
                                    "only constant assignment (::=) may alias types",
                                    node->location);
                                return nullptr;
                            }
                        }
                    } else {
                        init_expr = builder.make_identifier_reference(
                            scope,
                            init_symbol->qualified_symbol(),
                            nullptr);
                    }
                }
                if (init_expr->is_constant()) {
                    init = builder.make_initializer(scope, init_expr);
                }
            }
        }

        auto new_identifier = builder.make_identifier(scope, symbol, init);
        apply_attributes(context, new_identifier, node);
        if (init_expr != nullptr) {
            if (init == nullptr)
                init_expr->parent_element(new_identifier);
        }

        if (type_find_result.type == nullptr) {
            if (init_expr != nullptr) {
                infer_type_result_t infer_type_result {};
                if (!init_expr->infer_type(_session, infer_type_result)) {
                    // XXX: error
                    return nullptr;
                }
                type_find_result.type = infer_type_result.inferred_type;
                new_identifier->type(type_find_result.type);
                new_identifier->inferred_type(type_find_result.type != nullptr);
            }

            if (type_find_result.type == nullptr) {
                new_identifier->type(builder.make_unknown_type_from_find_result(
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
                context,
                dynamic_cast<procedure_type*>(init->expression()),
                source_node.get());
        }

        if (init == nullptr
        &&  init_expr == nullptr
        &&  new_identifier->type() == nullptr) {
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

                // XXX: need to refactor this so that the binary_op is returned
                //      along with the identifier so that the statement function can
                //      be solely responsible for adding new statements to the scope.
                auto assign_bin_op = builder.make_binary_operator(
                    scope,
                    operator_type_t::assignment,
                    new_identifier,
                    init_expr);
                auto statement = builder.make_statement(
                    scope,
                    label_list_t{},
                    assign_bin_op);
                add_expression_to_scope(scope, statement);
                statement->parent_element(scope);
            }
        }

        return new_identifier;
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
        auto expression = evaluate(context.node->lhs.get());
        auto directive_element = _session.builder().make_directive(
            _session.scope_manager().current_scope(),
            context.node->token.value,
            expression);
        directive_element->location(context.node->location);
        apply_attributes(context, directive_element, context.node);
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
            auto expr = evaluate(
                (*it).get(),
                context.default_block_type);
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
        result.element = _session.builder().make_bool(
            _session.scope_manager().current_scope(),
            context.node->token.as_bool());
        result.element->location(context.node->location);
        return true;
    }

    bool ast_evaluator::namespace_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        result.element = _session.builder().make_namespace(
            _session.scope_manager().current_scope(),
            evaluate(context.node->rhs.get(), context.default_block_type));
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
        auto args = _session.builder().make_argument_list(_session.scope_manager().current_scope());
        for (const auto& arg_node : context.node->children) {
            auto arg = resolve_symbol_or_evaluate(context, arg_node.get());
            args->add(arg);
            arg->parent_element(args);
        }
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

    bool ast_evaluator::binary_operator(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto it = s_binary_operators.find(context.node->token.type);
        if (it == s_binary_operators.end())
            return false;
        auto lhs = resolve_symbol_or_evaluate(context, context.node->lhs.get());
        auto rhs = resolve_symbol_or_evaluate(context, context.node->rhs.get());
        result.element = _session.builder().make_binary_operator(
            _session.scope_manager().current_scope(),
            it->second,
            lhs,
            rhs);
        return true;
    }

    bool ast_evaluator::cast_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        qualified_symbol_t type_name {
            .name = context.node->lhs->lhs->children[0]->token.value
        };
        auto type = scope_manager.find_type(type_name);
        if (type == nullptr) {
            _session.error(
                "P002",
                fmt::format("unknown type '{}'.", type_name.name),
                context.node->lhs->lhs->location);
            return false;
        }
        auto cast_element = builder.make_cast(
            scope_manager.current_scope(),
            builder.make_type_reference(scope_manager.current_scope(), type_name, type),
            resolve_symbol_or_evaluate(context, context.node->rhs.get()));
        cast_element->location(context.node->location);
        cast_element->type_location(context.node->lhs->lhs->location);
        result.element = cast_element;
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

    bool ast_evaluator::basic_block(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& scope_manager = _session.scope_manager();
        auto active_scope = scope_manager.push_new_block(context.default_block_type);

        for (auto it = context.node->children.begin();
                 it != context.node->children.end();
                 ++it) {
            auto current_node = *it;
            auto expr = evaluate(
                current_node.get(),
                context.default_block_type);
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

        qualified_symbol_t qualified_symbol {};
        builder.make_qualified_symbol(qualified_symbol, context.node->lhs.get());

        compiler::argument_list* args = nullptr;
        auto expr = evaluate(context.node->rhs.get());
        if (expr != nullptr) {
            args = dynamic_cast<compiler::argument_list*>(expr);
        }

        auto intrinsic = compiler::intrinsic::intrinsic_for_call(
            _session,
            scope_manager.current_scope(),
            args,
            qualified_symbol);
        if (intrinsic != nullptr) {
            result.element = intrinsic;
            return true;
        }

        auto proc_identifier = scope_manager.find_identifier(qualified_symbol);
        result.element = builder.make_procedure_call(
            scope_manager.current_scope(),
            builder.make_identifier_reference(
                scope_manager.current_scope(),
                qualified_symbol,
                proc_identifier),
            args);
        result.element->location(context.node->location);

        return true;
    }

    bool ast_evaluator::statement(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        label_list_t labels {};

        if (context.node->lhs != nullptr) {
            for (const auto& label : context.node->lhs->children) {
                labels.push_back(builder.make_label(
                    scope_manager.current_scope(),
                    label->token.value));
            }
        }

        auto expr = evaluate(context.node->rhs.get());
        if (expr == nullptr)
            return false;

        if (expr->element_type() == element_type_t::symbol) {
            type_find_result_t find_type_result {};
            scope_manager.find_identifier_type(
                find_type_result,
                context.node->rhs->rhs);
            expr = add_identifier_to_scope(
                context,
                dynamic_cast<compiler::symbol_element*>(expr),
                find_type_result,
                nullptr,
                0);
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
        auto enum_type = builder.make_enum_type(
            active_scope,
            builder.make_block(active_scope, element_type_t::block));
        active_scope->types().add(enum_type);
        add_composite_type_fields(
            context,
            enum_type,
            context.node->rhs.get());
        if (!enum_type->initialize(_session))
            return false;
        result.element = enum_type;

        return true;
    }

    bool ast_evaluator::struct_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        auto active_scope = scope_manager.current_scope();
        auto struct_type = builder.make_struct_type(
            active_scope,
            builder.make_block(active_scope, element_type_t::block));
        active_scope->types().add(struct_type);
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
        auto union_type = builder.make_union_type(
            active_scope,
            builder.make_block(active_scope, element_type_t::block));
        active_scope->types().add(union_type);
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
        auto predicate = evaluate(context.node->lhs.get());
        auto true_branch = evaluate(context.node->children[0].get());
        auto false_branch = evaluate(context.node->rhs.get());
        result.element = _session.builder().make_if(
            _session.scope_manager().current_scope(),
            predicate,
            true_branch,
            false_branch);
        return true;
    }

    bool ast_evaluator::proc_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        auto active_scope = scope_manager.current_scope();
        auto block_scope = builder.make_block(
            active_scope,
            element_type_t::proc_type_block);
        auto proc_type = builder.make_procedure_type(active_scope, block_scope);
        active_scope->types().add(proc_type);

        auto count = 0;
        for (const auto& type_node : context.node->lhs->children) {
            switch (type_node->type) {
                case syntax::ast_node_types_t::symbol: {
                    auto return_identifier = builder.make_identifier(
                        block_scope,
                        builder.make_symbol(block_scope, fmt::format("_{}", count++)),
                        nullptr);
                    return_identifier->usage(identifier_usage_t::stack);
                    return_identifier->type(scope_manager.find_type(qualified_symbol_t {
                        .name = type_node->children[0]->token.value
                    }));
                    auto new_field = builder.make_field(proc_type, block_scope, return_identifier);
                    proc_type->returns().add(new_field);
                    break;
                }
                default: {
                    break;
                }
            }
        }

        for (const auto& param_node : context.node->rhs->children) {
            switch (param_node->type) {
                case syntax::ast_node_types_t::assignment: {
                    element_list_t list {};
                    auto success = add_assignments_to_scope(context, param_node.get(), list, block_scope);
                    if (success) {
                        auto param_identifier = dynamic_cast<compiler::identifier*>(list.front());
                        param_identifier->usage(identifier_usage_t::stack);
                        auto field = builder.make_field(proc_type, block_scope, param_identifier);
                        proc_type->parameters().add(field);
                    } else {
                        return false;
                    }
                    break;
                }
                case syntax::ast_node_types_t::symbol: {
                    auto param_identifier = declare_identifier(context, param_node.get(), block_scope);
                    if (param_identifier != nullptr) {
                        param_identifier->usage(identifier_usage_t::stack);
                        auto field = builder.make_field(proc_type, block_scope, param_identifier);
                        proc_type->parameters().add(field);
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
        element_list_t list {};
        auto success = add_assignments_to_scope(context, context.node, list, nullptr);
        if (success)
            result.element = list.front();
        return success;
    }

    bool ast_evaluator::transmute_expression(
            evaluator_context_t& context,
            evaluator_result_t& result) {
        auto& builder = _session.builder();
        auto& scope_manager = _session.scope_manager();

        qualified_symbol_t type_name {
            .name = context.node->lhs->lhs->children[0]->token.value
        };
        auto type = scope_manager.find_type(type_name);
        if (type == nullptr) {
            _session.error(
                "P002",
                fmt::format("unknown type '{}'.", type_name.name),
                context.node->lhs->lhs->location);
            return false;
        }
        auto transmute_element = builder.make_transmute(
            scope_manager.current_scope(),
            builder.make_type_reference(scope_manager.current_scope(), type_name, type),
            resolve_symbol_or_evaluate(context, context.node->rhs.get()));
        transmute_element->location(context.node->location);
        transmute_element->type_location(context.node->lhs->lhs->location);
        result.element = transmute_element;
        return true;
    }

    bool ast_evaluator::add_assignments_to_scope(
            const evaluator_context_t& context,
            const syntax::ast_node_t* node,
            element_list_t& identifiers,
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

            qualified_symbol_t qualified_symbol {};
            builder.make_qualified_symbol(qualified_symbol, target_symbol.get());
            auto existing_identifier = scope_manager.find_identifier(qualified_symbol, scope);
            if (existing_identifier != nullptr) {
                if (existing_identifier->symbol()->is_constant()) {
                    _session.error(
                        "P028",
                        "constant variables cannot be modified.",
                        target_symbol->location);
                    return false;
                }

                auto rhs = evaluate_in_scope(context, source_list->children[i].get(), scope);
                if (rhs == nullptr)
                    return false;
                auto binary_op = builder.make_binary_operator(
                    scope_manager.current_scope(),
                    operator_type_t::assignment,
                    existing_identifier,
                    rhs);
                apply_attributes(context, binary_op, node);
                identifiers.emplace_back(binary_op);
            } else {
                auto lhs = evaluate_in_scope(context, target_symbol.get(), scope);
                auto symbol = dynamic_cast<compiler::symbol_element*>(lhs);
                symbol->constant(is_constant_assignment);

                type_find_result_t find_type_result {};
                scope_manager.find_identifier_type(
                    find_type_result,
                    target_symbol->rhs,
                    scope);
                auto new_identifier = add_identifier_to_scope(
                    context,
                    symbol,
                    find_type_result,
                    node,
                    i,
                    scope);
                if (new_identifier == nullptr)
                    return false;
                identifiers.emplace_back(new_identifier);
            }
        }

        return true;
    }

    compiler::identifier* ast_evaluator::declare_identifier(
            const evaluator_context_t& context,
            const syntax::ast_node_t* node,
            compiler::block* scope) {
        auto& scope_manager = _session.scope_manager();

        auto element = evaluate_in_scope(context, node, scope);
        auto symbol = dynamic_cast<compiler::symbol_element*>(element);

        type_find_result_t type_find_result {};
        scope_manager.find_identifier_type(
            type_find_result,
            node->rhs,
            scope);

        return add_identifier_to_scope(
            context,
            symbol,
            type_find_result,
            nullptr,
            0,
            scope);
    }

};