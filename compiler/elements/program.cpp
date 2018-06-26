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
#include "expression.h"
#include "identifier.h"
#include "if_element.h"
#include "array_type.h"
#include "initializer.h"
#include "string_type.h"
#include "numeric_type.h"
#include "float_literal.h"
#include "string_literal.h"
#include "unary_operator.h"
#include "composite_type.h"
#include "procedure_type.h"
#include "return_element.h"
#include "procedure_call.h"
#include "namespace_type.h"
#include "boolean_literal.h"
#include "binary_operator.h"
#include "integer_literal.h"
#include "namespace_element.h"
#include "procedure_instance.h"

namespace basecode::compiler {

    program::program() : element(nullptr, element_type_t::program) {
    }

    program::~program() {
        for (auto element : _elements)
            delete element.second;
        _elements.clear();
    }

    compiler::block* program::block() {
        return _block;
    }

    element* program::evaluate(
            common::result& r,
            const syntax::ast_node_shared_ptr& node) {
        if (node == nullptr)
            return nullptr;

        switch (node->type) {
            case syntax::ast_node_types_t::symbol: {
                if (node->has_type_identifier()) {
                    return add_identifier_to_scope(r, node, nullptr);
                } else {
                    return find_identifier(node);
                }
            }
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
                auto scope_block = push_new_block();

                if (node->type == syntax::ast_node_types_t::program) {
                    _block = scope_block;
                    initialize_core_types();
                } else {
                    current_scope()->blocks().push_back(scope_block);
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
                const auto& assignment_target_list = node->lhs;

                identifier_list_t list {};
                for (const auto& symbol : assignment_target_list->children) {
                    auto new_identifier = add_identifier_to_scope(r, symbol, node->rhs);
                    list.push_back(new_identifier);
                }

                // XXX: handle proper multi-assignment

                return list.front();
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
            case syntax::ast_node_types_t::string_literal: {
                return make_string(node->token.value);
            }
            case syntax::ast_node_types_t::number_literal: {
                switch (node->token.number_type) {
                    case syntax::number_types_t::integer: {
                        uint64_t value;
                        if (node->token.parse(value) == syntax::conversion_result_t::success)
                            return make_integer(value);
                        // XXX: need to handle conversion failures
                    }
                    case syntax::number_types_t::floating_point: {
                        double value;
                        if (node->token.parse(value) == syntax::conversion_result_t::success)
                            return make_float(value);
                        // XXX: need to handle conversion failures
                    }
                    default:
                        break;
                }
                return nullptr;
            }
            case syntax::ast_node_types_t::boolean_literal: {
                return make_bool(node->token.as_bool());
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
            case syntax::ast_node_types_t::proc_call: {
                auto proc_identifier = find_identifier(node->lhs);
                if (proc_identifier != nullptr) {
                    return make_procedure_call(
                        proc_identifier->type(),
                        evaluate(r, node->rhs));
                }
                return nullptr;
            }
            case syntax::ast_node_types_t::proc_expression: {
                auto block_scope = make_block();
                auto proc_type = make_procedure_type(block_scope);
                current_scope()->types().add(proc_type);

                push_scope(block_scope);

                auto count = 0;
                for (const auto& type_node : node->lhs->children) {
                    switch (type_node->type) {
                        case syntax::ast_node_types_t::symbol: {
                            auto return_identifier = make_identifier(
                                fmt::format("_{}", count++),
                                nullptr);
                            return_identifier->type(find_type(type_node->children[0]->token.value));
                            proc_type->returns().add(make_field(return_identifier));
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
                            const auto& symbol_node = param_node->lhs->children[0];
                            auto param_identifier = add_identifier_to_scope(
                                r,
                                symbol_node,
                                param_node->rhs);
                            auto field = make_field(param_identifier);
                            proc_type->parameters().add(field);
                            break;
                        }
                        case syntax::ast_node_types_t::symbol: {
                            auto param_identifier = add_identifier_to_scope(
                                r,
                                param_node,
                                nullptr);
                            auto field = make_field(param_identifier);
                            proc_type->parameters().add(field);
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                }

//                if (!node->children.empty()) {
//                    for (const auto& child_node : node->children) {
//                        switch (child_node->type) {
//                            case syntax::ast_node_types_t::attribute: {
//                                proc_type->attributes().add(make_attribute(
//                                    child_node->token.value,
//                                    evaluate(r, child_node->lhs)));
//                                break;
//                            }
//                            case syntax::ast_node_types_t::basic_block: {
//                                auto basic_block = dynamic_cast<compiler::block*>(evaluate(
//                                    r,
//                                    child_node));
//                                proc_type->instances().push_back(make_procedure_instance(
//                                    proc_type,
//                                    basic_block));
//                            }
//                            default:
//                                break;
//                        }
//                    }
//                }

                pop_scope();

                return proc_type;
            }
            case syntax::ast_node_types_t::enum_expression: {
                auto enum_type = make_enum();
                current_scope()->types().add(enum_type);
                add_enum_fields(r, enum_type, node->rhs);
                return enum_type;
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
                auto union_type = make_union_type();
                current_scope()->types().add(union_type);
                add_union_fields(r, union_type, node->rhs);
                return union_type;
            }
            case syntax::ast_node_types_t::struct_expression: {
                auto struct_type = make_struct_type();
                current_scope()->types().add(struct_type);
                add_struct_fields(r, struct_type, node->rhs);
                return struct_type;
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
                return make_namespace(evaluate(r, node->rhs));
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

    compiler::block* program::pop_scope() {
        if (_scope_stack.empty())
            return nullptr;
        auto top = _scope_stack.top();
        _scope_stack.pop();
        return top;
    }

    field* program::make_field(compiler::identifier* identifier) {
        auto field = new compiler::field(current_scope(), identifier);
        _elements.insert(std::make_pair(field->id(), field));
        return field;
    }

    const element_map_t& program::elements() const {
        return _elements;
    }

    compiler::block* program::push_new_block() {
        auto type = make_block();
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

    compiler::block* program::current_scope() const {
        if (_scope_stack.empty())
            return nullptr;
        return _scope_stack.top();
    }

    void program::initialize_core_types() {
        auto& scope_types = current_scope()->types();
        scope_types.add(make_any_type());
        scope_types.add(make_string_type());
        scope_types.add(make_namespace_type());
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
            initializer* expr,
            compiler::block* block_scope) {
        auto identifier = new compiler::identifier(
            block_scope == nullptr ? current_scope() : block_scope,
            name,
            expr);
        _elements.insert(std::make_pair(identifier->id(), identifier));
        return identifier;
    }

    void program::add_procedure_instance(
            common::result& r,
            compiler::procedure_type* proc_type,
            const syntax::ast_node_shared_ptr& node) {
        if (node->children.empty())
            return;

        for (const auto& child_node : node->children) {
            switch (child_node->type) {
                case syntax::ast_node_types_t::attribute: {
                    proc_type->attributes().add(make_attribute(
                        child_node->token.value,
                        evaluate(r, child_node->lhs)));
                    break;
                }
                case syntax::ast_node_types_t::basic_block: {
                    auto basic_block = dynamic_cast<compiler::block*>(evaluate(
                        r,
                        child_node));
                    proc_type->instances().push_back(make_procedure_instance(
                        proc_type,
                        basic_block));
                }
                default:
                    break;
            }
        }
    }

    composite_type* program::make_enum() {
        auto type = new compiler::composite_type(
            current_scope(),
            composite_types_t::enum_type,
            fmt::format("__enum_{}__", common::id_pool::instance()->allocate()));
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    initializer* program::make_initializer(
            element* expr,
            compiler::block* block_scope) {
        auto initializer = new compiler::initializer(
            block_scope == nullptr ? current_scope() : block_scope,
            expr);
        _elements.insert(std::make_pair(initializer->id(), initializer));
        return initializer;
    }

    return_element* program::make_return() {
        auto return_element = new compiler::return_element(current_scope());
        _elements.insert(std::make_pair(return_element->id(), return_element));
        return return_element;
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

    namespace_element* program::make_namespace(
            element* expr,
            compiler::block* block_scope) {
        auto ns = new compiler::namespace_element(
            block_scope == nullptr ? current_scope() : block_scope,
            expr);
        _elements.insert(std::make_pair(ns->id(), ns));
        return ns;
    }

    composite_type* program::make_union_type() {
        auto type = new compiler::composite_type(
            current_scope(),
            composite_types_t::union_type,
            fmt::format("__union_{}__", common::id_pool::instance()->allocate()));
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    composite_type* program::make_struct_type() {
        auto type = new compiler::composite_type(
            current_scope(),
            composite_types_t::struct_type,
            fmt::format("__struct_{}__", common::id_pool::instance()->allocate()));
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    procedure_call* program::make_procedure_call(
            compiler::type* procedure_type,
            element* expr) {
        auto proc_call = new compiler::procedure_call(
            current_scope(),
            procedure_type,
            expr);
        _elements.insert(std::make_pair(proc_call->id(), proc_call));
        return proc_call;
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

    namespace_type* program::make_namespace_type() {
        auto type = new compiler::namespace_type(current_scope());
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    element* program::find_element(common::id_t id) {
        auto it = _elements.find(id);
        if (it != _elements.end())
            return it->second;
        return nullptr;
    }

    boolean_literal* program::make_bool(bool value) {
        auto boolean_literal = new compiler::boolean_literal(current_scope(), value);
        _elements.insert(std::make_pair(boolean_literal->id(), boolean_literal));
        return boolean_literal;
    }

    void program::push_scope(compiler::block* block) {
        _scope_stack.push(block);
    }

    type* program::find_type(const std::string& name) {
        auto scope = current_scope();
        while (scope != nullptr) {
            auto type = scope->types().find(name);
            if (type != nullptr)
                return type;
            scope = dynamic_cast<compiler::block*>(scope->parent());
        }
        return nullptr;
    }

    float_literal* program::make_float(double value) {
        auto literal = new compiler::float_literal(current_scope(), value);
        _elements.insert(std::make_pair(literal->id(), literal));
        return literal;
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

    integer_literal* program::make_integer(uint64_t value) {
        auto literal = new compiler::integer_literal(current_scope(), value);
        _elements.insert(std::make_pair(literal->id(), literal));
        return literal;
    }

    compiler::identifier* program::add_identifier_to_scope(
            common::result& r,
            const syntax::ast_node_shared_ptr& symbol,
            const syntax::ast_node_shared_ptr& rhs) {
        auto namespace_type = find_type("namespace");

        compiler::type* identifier_type = nullptr;
        if (symbol->rhs != nullptr) {
            auto type_name = symbol->rhs->token.value;
            if (!type_name.empty()) {
                identifier_type = find_type(type_name);

                if (symbol->rhs->is_array()) {
                    auto array_type = find_array_type(identifier_type, 0);
                    if (array_type == nullptr)
                        array_type = make_array_type(identifier_type, 0);
                    identifier_type = array_type;
                }
            }
        }

        auto scope = symbol->is_qualified_symbol() ? block() : current_scope();

        for (size_t i = 0; i < symbol->children.size() - 1; i++) {
            const auto& symbol_node = symbol->children[i];
            auto ident = scope->identifiers().find(symbol_node->token.value);
            if (ident == nullptr) {
                auto new_scope = make_block(scope);
                auto ns_identifier = make_identifier(
                    symbol_node->token.value,
                    make_initializer(
                        make_namespace(new_scope, new_scope),
                        new_scope),
                    new_scope);
                ns_identifier->type(namespace_type);
                ns_identifier->inferred_type(true);
                scope->identifiers().add(ns_identifier);
                scope = new_scope;
            } else {
                auto expr = ident->initializer()->expression();
                if (expr->element_type() == element_type_t::namespace_e) {
                    auto ns = dynamic_cast<namespace_element*>(expr);
                    scope = dynamic_cast<compiler::block*>(ns->expression());
                } else {
                    // XXX: what should really be happening here
                    //      if the scope isn't a namespace, is that an error?
                    break;
                }
            }
        }

        push_scope(scope);

        const auto& final_symbol = symbol->children.back();

        auto init = (compiler::initializer*)nullptr;
        if (rhs != nullptr) {
            auto init_expression = evaluate(r, rhs);
            if (init_expression != nullptr)
                init = make_initializer(init_expression);
        }

        auto new_identifier = make_identifier(
            final_symbol->token.value,
            init);

        if (identifier_type == nullptr && init != nullptr) {

        }

        new_identifier->type(identifier_type);

        new_identifier->constant(symbol->is_constant_expression());
        scope->identifiers().add(new_identifier);

        // XXX: if identifier_type is a procedure, we need to call add_procedure_instances

        pop_scope();

        return new_identifier;
    }

    void program::add_enum_fields(
            common::result& r,
            compiler::composite_type* enum_type,
            const syntax::ast_node_shared_ptr& block) {
        // XXX: hard coded for now, need to add type parameter support to enum literal
        auto u32_type = find_type("u32");
        for (const auto& child : block->children) {
            if (child->type != syntax::ast_node_types_t::statement) {
                // XXX: this is an error!
                break;
            }
            auto expr_node = child->rhs;
            switch (expr_node->type) {
                case syntax::ast_node_types_t::assignment: {
                    compiler::type* field_type = nullptr;
                    if (expr_node->lhs->rhs != nullptr) {
                        field_type = find_type(expr_node->lhs->rhs->token.value);
                    }
                    auto field_identifier = make_identifier(
                        expr_node->lhs->children[0]->token.value,
                        make_initializer(evaluate(r, expr_node->rhs)));
                    field_identifier->type(field_type);
                    enum_type->fields().add(make_field(field_identifier));
                    break;
                }
                case syntax::ast_node_types_t::symbol: {
                    auto field_identifier = make_identifier(
                        expr_node->children[0]->token.value,
                        nullptr);
                    field_identifier->type(u32_type);
                    enum_type->fields().add(make_field(field_identifier));
                    break;
                }
                default:
                    break;
            }
        }
    }

    void program::add_struct_fields(
            common::result& r,
            compiler::composite_type* struct_type,
            const syntax::ast_node_shared_ptr& block) {
        for (const auto& child : block->children) {
            if (child->type != syntax::ast_node_types_t::statement) {
                // XXX: this is an error!
                break;
            }
            auto expr_node = child->rhs;
            switch (expr_node->type) {
                case syntax::ast_node_types_t::assignment: {
                    compiler::type* field_type = nullptr;
                    if (expr_node->lhs->rhs != nullptr) {
                        field_type = find_type(expr_node->lhs->rhs->token.value);
                    }
                    auto field_identifier = make_identifier(
                        expr_node->lhs->children[0]->token.value,
                        make_initializer(evaluate(r, expr_node->rhs)));
                    field_identifier->type(field_type);
                    struct_type->fields().add(make_field(field_identifier));
                    break;
                }
                case syntax::ast_node_types_t::symbol: {
                    compiler::type* field_type = nullptr;
                    if (expr_node->rhs != nullptr)
                        field_type = find_type(expr_node->rhs->token.value);
                    auto field_identifier = make_identifier(
                        expr_node->children[0]->token.value,
                        nullptr);
                    field_identifier->type(field_type);
                    struct_type->fields().add(make_field(field_identifier));
                    break;
                }
                default:
                    break;
            }
        }
    }

    void program::add_union_fields(
            common::result& r,
            compiler::composite_type* union_type,
            const syntax::ast_node_shared_ptr& block) {
        for (const auto& child : block->children) {
            if (child->type != syntax::ast_node_types_t::statement) {
                // XXX: this is an error!
                break;
            }
            auto expr_node = child->rhs;
            switch (expr_node->type) {
                case syntax::ast_node_types_t::assignment: {
                    compiler::type* field_type = nullptr;
                    if (expr_node->lhs->rhs != nullptr) {
                        field_type = find_type(expr_node->lhs->rhs->token.value);
                    }
                    auto field_identifier = make_identifier(
                        expr_node->lhs->children[0]->token.value,
                        make_initializer(evaluate(r, expr_node->rhs)));
                    field_identifier->type(field_type);
                    union_type->fields().add(make_field(field_identifier));
                    break;
                }
                case syntax::ast_node_types_t::symbol: {
                    compiler::type* field_type = nullptr;
                    if (expr_node->rhs != nullptr)
                        field_type = find_type(expr_node->rhs->token.value);
                    auto field_identifier = make_identifier(
                        expr_node->children[0]->token.value,
                        nullptr);
                    field_identifier->type(field_type);
                    union_type->fields().add(make_field(field_identifier));
                    break;
                }
                default:
                    break;
            }
        }
    }

    array_type* program::make_array_type(compiler::type* entry_type, size_t size) {
        auto type = new compiler::array_type(
            current_scope(),
            fmt::format("__array_{}_{}__", entry_type->name(), size),
            entry_type);
        type->size(size);
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    cast* program::make_cast(compiler::type* type, element* expr) {
        auto cast = new compiler::cast(current_scope(), type, expr);
        _elements.insert(std::make_pair(cast->id(), cast));
        return cast;
    }

    string_literal* program::make_string(const std::string& value) {
        auto literal = new compiler::string_literal(current_scope(), value);
        _elements.insert(std::make_pair(literal->id(), literal));
        return literal;
    }

    compiler::block* program::make_block(compiler::block* parent_scope) {
        auto type = new compiler::block(
            parent_scope == nullptr ? current_scope() : parent_scope);
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    statement* program::make_statement(label_list_t labels, element* expr) {
        auto statement = new compiler::statement(current_scope(), expr);
        for (auto label : labels)
            statement->labels().push_back(label);
        _elements.insert(std::make_pair(statement->id(), statement));
        return statement;
    }

    procedure_type* program::make_procedure_type(compiler::block* block_scope) {
        auto type = new compiler::procedure_type(
            current_scope(),
            block_scope,
            fmt::format("__proc_{}__", common::id_pool::instance()->allocate()));
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    compiler::type* program::find_array_type(compiler::type* entry_type, size_t size) {
        return find_type(fmt::format("__array_{}_{}__", entry_type->name(), size));
    }

    compiler::identifier* program::find_identifier(const syntax::ast_node_shared_ptr& node) {
        auto ident = (identifier*) nullptr;
        if (node->is_qualified_symbol()) {
            auto block_scope = block();
            for (const auto& symbol_node : node->children) {
                ident = block_scope->identifiers().find(symbol_node->token.value);
                if (ident == nullptr || ident->initializer() == nullptr)
                    return nullptr;
                auto expr = ident->initializer()->expression();
                if (expr->element_type() == element_type_t::namespace_e) {
                    auto ns = dynamic_cast<namespace_element*>(expr);
                    block_scope = dynamic_cast<compiler::block*>(ns->expression());
                } else {
                    break;
                }
            }
        } else {
            const auto& symbol_part = node->children[0];
            auto block_scope = current_scope();
            while (block_scope != nullptr) {
                fmt::print("block_scope: {}\n", block_scope->id());
                block_scope->identifiers().dump();
                fmt::print("-----------------------------------------\n");
                ident = block_scope->identifiers().find(symbol_part->token.value);
                if (ident != nullptr)
                    return ident;
                block_scope = dynamic_cast<compiler::block*>(block_scope->parent());
            }
            return nullptr;
        }
        return ident;
    }

};