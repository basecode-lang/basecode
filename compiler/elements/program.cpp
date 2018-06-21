// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#include <fmt/format.h>
#include "type.h"
#include "cast.h"
#include "label.h"
#include "alias.h"
#include "comment.h"
#include "program.h"
#include "any_type.h"
#include "attribute.h"
#include "directive.h"
#include "statement.h"
#include "identifier.h"
#include "if_element.h"
#include "initializer.h"
#include "string_type.h"
#include "numeric_type.h"
#include "unary_operator.h"
#include "composite_type.h"
#include "procedure_type.h"
#include "return_element.h"
#include "binary_operator.h"
#include "procedure_instance.h"

namespace basecode::compiler {

    program::program() : block(nullptr, element_type_t::program) {
    }

    program::~program() {
        for (auto element : _elements)
            delete element.second;
        _elements.clear();
    }

    element* program::evaluate(
            common::result& r,
            const syntax::ast_node_shared_ptr& node) {
        if (node == nullptr)
            return nullptr;

        switch (node->type) {
            case syntax::ast_node_types_t::attribute: {
                return make_attribute(
                    node->token.value,
                    evaluate(r, node->rhs));
            }
            case syntax::ast_node_types_t::directive: {
                return make_directive(
                    node->token.value,
                    evaluate(r, node->lhs));
            }
            case syntax::ast_node_types_t::program:
            case syntax::ast_node_types_t::basic_block: {
                make_new_block();

                if (node->type == syntax::ast_node_types_t::program) {
                    initialize_core_types();
                }

                for (auto it = node->children.begin();
                     it != node->children.end();
                     ++it) {
                    auto expr = evaluate(r, *it);
                    switch (expr->element_type()) {
                        case element_type_t::comment:
                            // XXX: this will need to be revisited
                            current_scope()
                                ->comments()
                                    .push_back(dynamic_cast<comment*>(expr));
                            break;
                        case element_type_t::attribute:
                            current_scope()
                                ->attributes()
                                    .add(dynamic_cast<attribute*>(expr));
                            break;
                        case element_type_t::statement: {
                            current_scope()
                                ->statements()
                                    .push_back(dynamic_cast<statement*>(expr));
                            break;
                        }
                        default:
                            break;
                    }
                }

                return pop_scope();
            }
            case syntax::ast_node_types_t::statement: {
                label_list_t labels {};

                if (node->lhs != nullptr) {
                    for (const auto& label : node->lhs->children) {
                        labels.push_back(make_label(label->token.value));
                    }
                }

                return make_statement(
                    labels,
                    evaluate(r, node->rhs));
            }
            case syntax::ast_node_types_t::expression: {
                return make_expression(evaluate(r, node->lhs));
            }
            case syntax::ast_node_types_t::assignment: {
                return make_binary_operator(
                    operator_type_t::assignment,
                    evaluate(r, node->lhs),
                    evaluate(r, node->rhs));
            }
            case syntax::ast_node_types_t::line_comment: {
                return make_comment(
                    comment_type_t::line,
                    node->token.value);
            }
            case syntax::ast_node_types_t::block_comment: {
                return make_comment(
                    comment_type_t::block,
                    node->token.value);
            }
            case syntax::ast_node_types_t::else_expression: {
                return evaluate(r, node->children[0]);
            }
            case syntax::ast_node_types_t::if_expression:
            case syntax::ast_node_types_t::elseif_expression: {
                auto predicate = evaluate(r, node->lhs);
                auto true_branch = evaluate(r, node->children[0]);
                auto false_branch = evaluate(r, node->rhs);
                return make_if(predicate, true_branch, false_branch);
            }
            case syntax::ast_node_types_t::unary_operator: {
                auto it = s_unary_operators.find(node->token.type);
                if (it == s_unary_operators.end())
                    return nullptr;
                return make_unary_operator(
                    it->second,
                    evaluate(r, node->rhs));
            }
            case syntax::ast_node_types_t::binary_operator: {
                auto it = s_binary_operators.find(node->token.type);
                if (it == s_binary_operators.end())
                    return nullptr;
                return make_binary_operator(
                    it->second,
                    evaluate(r, node->lhs),
                    evaluate(r, node->rhs));
            }
            case syntax::ast_node_types_t::proc_expression: {
                auto proc_type = make_procedure_type();
                current_scope()->types().add(proc_type);

                auto count = 0;
                for (const auto& type_node : node->lhs->children) {
                    switch (type_node->type) {
                        case syntax::ast_node_types_t::qualified_symbol_reference: {
                            // XXX: I am a horrible human being
                            auto total_hack_fix_me = type_node->lhs->children[0];
                            proc_type->returns().add(make_field(
                                fmt::format("_{}", count++),
                                find_type(total_hack_fix_me->token.value),
                                nullptr));
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                }

                for (const auto& type_node : node->rhs->children) {
                    proc_type->parameters().add(make_field(
                        type_node->token.value,
                        find_type(type_node->lhs->token.value),
                        type_node->rhs != nullptr ? make_initializer(evaluate(r, type_node->rhs)) : nullptr));
                }

                if (!node->children.empty()) {
                    for (const auto& child_node : node->children) {
                        switch (child_node->type) {
                            case syntax::ast_node_types_t::attribute: {
                                proc_type->attributes().add(make_attribute(
                                    child_node->token.value,
                                    evaluate(r, child_node->lhs)));
                                break;
                            }
                            case syntax::ast_node_types_t::basic_block: {
                                proc_type->instances().push_back(make_procedure_instance(
                                    proc_type,
                                    dynamic_cast<block*>(evaluate(r, child_node))));
                            }
                            default:
                                break;
                        }
                    }
                }

                return proc_type;
            }
            case syntax::ast_node_types_t::enum_expression: {
                auto scope = current_scope();
                auto& scope_types = scope->types();

                auto type = make_enum();
                scope_types.add(type);

//                for (const auto& child : node->rhs->children) {
//                    auto field_element = evaluate(r, child);
//                    auto field = make_field("", nullptr, nullptr);
//                    type->fields().add(field);
//                }

                return type;
            }
            case syntax::ast_node_types_t::cast_expression: {
                auto type = find_type(node->lhs->token.value);
                if (type == nullptr) {
                    r.add_message(
                        "P002",
                        fmt::format("unknown type '{}'.", node->lhs->token.value),
                        true);
                }
                return make_cast(type, evaluate(r, node->rhs));
            }
            case syntax::ast_node_types_t::alias_expression: {
                return make_alias(evaluate(r, node->lhs));
            }
            case syntax::ast_node_types_t::union_expression: {
                auto scope = current_scope();
                auto& scope_types = scope->types();

                auto type = make_union();
                scope_types.add(type);

//                for (const auto& child : node->rhs->children) {
//                }

                return type;
            }
            case syntax::ast_node_types_t::struct_expression: {
                auto scope = current_scope();
                auto& scope_types = scope->types();

                auto type = make_struct();
                scope_types.add(type);

//                for (const auto& child : node->rhs->children) {
//                }

                return type;
            }
            case syntax::ast_node_types_t::return_statement: {
                auto return_element = make_return();
                for (const auto& arg_node : node->rhs->children) {
                    return_element->expressions().push_back(evaluate(r, arg_node));
                }
                return return_element;
            }
            case syntax::ast_node_types_t::constant_expression: {
                auto identifier = dynamic_cast<compiler::identifier*>(evaluate(r, node->rhs));
                if (identifier != nullptr)
                    identifier->constant(true);
                return identifier;
            }
            case syntax::ast_node_types_t::namespace_expression: {
                return evaluate(r, node->rhs);
            }
            case syntax::ast_node_types_t::qualified_symbol_reference: {
                break;
            }
            default: {
                break;
            }
        }

        return nullptr;
    }

    bool program::initialize(
            common::result& r,
            const syntax::ast_node_shared_ptr& root) {
        if (root->type != syntax::ast_node_types_t::program) {
            r.add_message(
                "P001",
                "The root AST node must be of type 'program'.",
                true);
            return false;
        }

        evaluate(r, root);

        return true;
    }

    block* program::pop_scope() {
        if (_scope_stack.empty())
            return nullptr;
        auto top = _scope_stack.top();
        _scope_stack.pop();
        return top;
    }

    field* program::make_field(
            const std::string& name,
            compiler::type* type,
            compiler::initializer* initializer) {
        auto field = new compiler::field(current_scope(), name, type, initializer);
        _elements.insert(std::make_pair(field->id(), field));
        return field;
    }

    block* program::make_new_block() {
        auto type = new block(current_scope());
        _elements.insert(std::make_pair(type->id(), type));
        push_scope(type);
        return type;
    }

    bool program::is_subtree_constant(
            const syntax::ast_node_shared_ptr& node) {
        if (node == nullptr)
            return false;

        switch (node->type) {
            case syntax::ast_node_types_t::expression: {
                return is_subtree_constant(node->lhs);
            }
            case syntax::ast_node_types_t::assignment: {
                return is_subtree_constant(node->rhs);
            }
            case syntax::ast_node_types_t::unary_operator: {
                return is_subtree_constant(node->rhs);
            }
            case syntax::ast_node_types_t::binary_operator: {
                return is_subtree_constant(node->lhs)
                       && is_subtree_constant(node->rhs);
            }
            case syntax::ast_node_types_t::basic_block:
            case syntax::ast_node_types_t::line_comment:
            case syntax::ast_node_types_t::null_literal:
            case syntax::ast_node_types_t::block_comment:
            case syntax::ast_node_types_t::number_literal:
            case syntax::ast_node_types_t::string_literal:
            case syntax::ast_node_types_t::boolean_literal:
            case syntax::ast_node_types_t::character_literal:
                return true;
            default:
                return false;
        }
    }

    any_type* program::make_any_type() {
        auto type = new any_type(current_scope());
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    block* program::current_scope() const {
        if (_scope_stack.empty())
            return nullptr;
        return _scope_stack.top();
    }

    void program::initialize_core_types() {
        auto& scope_types = current_scope()->types();
        scope_types.add(make_any_type());
        scope_types.add(make_string_type());
        scope_types.add(make_numeric_type("bool",    0,         1));
        scope_types.add(make_numeric_type("u8",      0,         UINT8_MAX));
        scope_types.add(make_numeric_type("u16",     0,         UINT16_MAX));
        scope_types.add(make_numeric_type("u32",     0,         UINT32_MAX));
        scope_types.add(make_numeric_type("u64",     0,         UINT64_MAX));
        scope_types.add(make_numeric_type("s8",      INT8_MIN,  INT8_MAX));
        scope_types.add(make_numeric_type("s16",     INT16_MIN, INT16_MAX));
        scope_types.add(make_numeric_type("s32",     INT32_MIN, INT32_MAX));
        scope_types.add(make_numeric_type("s64",     INT64_MIN, INT64_MAX));
        scope_types.add(make_numeric_type("f32",     0,         UINT32_MAX));
        scope_types.add(make_numeric_type("f64",     0,         UINT64_MAX));
        scope_types.add(make_numeric_type("address", 0,         UINTPTR_MAX));
    }

    if_element* program::make_if(
            element* predicate,
            element* true_branch,
            element* false_branch) {
        auto if_element = new compiler::if_element(
            current_scope(),
            predicate,
            true_branch,
            false_branch);
        _elements.insert(std::make_pair(if_element->id(), if_element));
        return if_element;
    }

    comment* program::make_comment(
            comment_type_t type,
            const std::string& value) {
        auto comment = new compiler::comment(current_scope(), type, value);
        _elements.insert(std::make_pair(comment->id(), comment));
        return comment;
    }

    directive* program::make_directive(
            const std::string& name,
            element* expr) {
        auto directive1 = new compiler::directive(current_scope(), name, expr);
        _elements.insert(std::make_pair(directive1->id(), directive1));
        return directive1;
    }

    attribute* program::make_attribute(
            const std::string& name,
            element* expr) {
        auto attr = new attribute(current_scope(), name, expr);
        _elements.insert(std::make_pair(attr->id(), attr));
        return attr;
    }

    identifier* program::make_identifier(
            const std::string& name,
            initializer* expr) {
        auto identifier = new compiler::identifier(current_scope(), name, expr);
        _elements.insert(std::make_pair(identifier->id(), identifier));
        return identifier;
    }

    composite_type* program::make_enum() {
        auto type = new compiler::composite_type(
            current_scope(),
            composite_types_t::enum_type,
            fmt::format("__enum_{}__", common::id_pool::instance()->allocate()));
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    composite_type* program::make_union() {
        auto type = new compiler::composite_type(
            current_scope(),
            composite_types_t::union_type,
            fmt::format("__union_{}__", common::id_pool::instance()->allocate()));
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    composite_type* program::make_struct() {
        auto type = new compiler::composite_type(
            current_scope(),
            composite_types_t::struct_type,
            fmt::format("__struct_{}__", common::id_pool::instance()->allocate()));
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    void program::push_scope(block* block) {
        _scope_stack.push(block);
    }

    return_element* program::make_return() {
        auto return_element = new compiler::return_element(current_scope());
        _elements.insert(std::make_pair(return_element->id(), return_element));
        return return_element;
    }

    element* program::find_element(id_t id) {
        auto it = _elements.find(id);
        if (it != _elements.end())
            return it->second;
        return nullptr;
    }

    numeric_type* program::make_numeric_type(
            const std::string& name,
            int64_t min,
            uint64_t max) {
        auto type = new compiler::numeric_type(current_scope(), name, min, max);
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    string_type* program::make_string_type() {
        auto type = new compiler::string_type(current_scope());
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    alias* program::make_alias(element* expr) {
        auto alias_type = new compiler::alias(current_scope(), expr);
        _elements.insert(std::make_pair(alias_type->id(), alias_type));
        return alias_type;
    }

    unary_operator* program::make_unary_operator(
            operator_type_t type,
            element* rhs) {
        auto unary_operator = new compiler::unary_operator(current_scope(), type, rhs);
        _elements.insert(std::make_pair(unary_operator->id(), unary_operator));
        return unary_operator;
    }

    binary_operator* program::make_binary_operator(
            operator_type_t type,
            element* lhs,
            element* rhs) {
        auto binary_operator = new compiler::binary_operator(current_scope(), type, lhs, rhs);
        _elements.insert(std::make_pair(binary_operator->id(), binary_operator));
        return binary_operator;
    }

    procedure_type* program::make_procedure_type() {
        // XXX: the name of the proc isn't correct here but it works temporarily.
        auto type = new compiler::procedure_type(
            current_scope(),
            fmt::format("__proc_{}__", common::id_pool::instance()->allocate()));
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    type* program::find_type(const std::string& name) {
        auto scope = current_scope();
        while (scope != nullptr) {
            auto type = scope->types().find(name);
            if (type != nullptr)
                return type;
            scope = dynamic_cast<block*>(scope->parent());
        }
        return nullptr;
    }

    procedure_instance* program::make_procedure_instance(
            compiler::type* procedure_type,
            compiler::block* scope) {
        auto instance = new compiler::procedure_instance(
            current_scope(),
            procedure_type,
            scope);
        _elements.insert(std::make_pair(instance->id(), instance));
        return instance;
    }

    expression* program::make_expression(element* expr) {
        auto expression = new compiler::expression(current_scope(), expr);
        _elements.insert(std::make_pair(expression->id(), expression));
        return expression;
    }

    label* program::make_label(const std::string& name) {
        auto label = new compiler::label(current_scope(), name);
        _elements.insert(std::make_pair(label->id(), label));
        return label;
    }

    initializer* program::make_initializer(element* expr) {
        auto initializer = new compiler::initializer(current_scope(), expr);
        _elements.insert(std::make_pair(initializer->id(), initializer));
        return initializer;
    }

    cast* program::make_cast(compiler::type* type, element* expr) {
        auto cast = new compiler::cast(current_scope(), type, expr);
        _elements.insert(std::make_pair(cast->id(), cast));
        return cast;
    }

    statement* program::make_statement(label_list_t labels, element* expr) {
        auto statement = new compiler::statement(current_scope(), expr);
        for (auto label : labels)
            statement->labels().push_back(label);
        _elements.insert(std::make_pair(statement->id(), statement));
        return statement;
    }

};