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
#include "import.h"
#include "comment.h"
#include "program.h"
#include "any_type.h"
#include "attribute.h"
#include "directive.h"
#include "statement.h"
#include "type_info.h"
#include "expression.h"
#include "identifier.h"
#include "if_element.h"
#include "array_type.h"
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
#include "boolean_literal.h"
#include "binary_operator.h"
#include "integer_literal.h"
#include "namespace_element.h"
#include "procedure_instance.h"

namespace basecode::compiler {

    program::program(vm::terp* terp) : element(nullptr, element_type_t::program),
                                       _assembler(terp),
                                       _terp(terp) {
    }

    program::~program() {
        for (auto element : _elements)
            delete element.second;
        _elements.clear();
    }

    element* program::evaluate(
            common::result& r,
            const syntax::ast_node_shared_ptr& node,
            element_type_t default_block_type) {
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
                    current_scope(),
                    node->token.value,
                    evaluate(r, node->lhs));
            }
            case syntax::ast_node_types_t::directive: {
                auto expression = evaluate(r, node->lhs);
                auto directive_element = make_directive(
                    current_scope(),
                    node->token.value,
                    expression);
                apply_attributes(r, directive_element, node);
                directive_element->evaluate(r, this);
                return directive_element;
            }
            case syntax::ast_node_types_t::module: {
                for (auto it = node->children.begin();
                     it != node->children.end();
                     ++it) {
                    add_expression_to_scope(_block, evaluate(r, *it));
                }
                return _block;
            }
            case syntax::ast_node_types_t::basic_block: {
                auto active_scope = push_new_block(default_block_type);

                for (auto it = node->children.begin();
                     it != node->children.end();
                     ++it) {
                    add_expression_to_scope(active_scope, evaluate(r, *it));
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

                return make_statement(
                    current_scope(),
                    labels,
                    evaluate(r, node->rhs));
            }
            case syntax::ast_node_types_t::expression: {
                return make_expression(
                    current_scope(),
                    evaluate(r, node->lhs));
            }
            case syntax::ast_node_types_t::assignment: {
                const auto& assignment_target_list = node->lhs;

                identifier_list_t list {};
                for (const auto& symbol : assignment_target_list->children) {
                    auto existing_identifier = find_identifier(symbol);
                    if (existing_identifier != nullptr) {
                        return make_binary_operator(
                            current_scope(),
                            operator_type_t::assignment,
                            existing_identifier,
                            evaluate(r, node->rhs));
                    } else {
                        auto new_identifier = add_identifier_to_scope(
                            r,
                            symbol,
                            node->rhs);
                        list.push_back(new_identifier);
                    }
                }

                // XXX: handle proper multi-assignment

                return list.front();
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
                return evaluate(r, node->children[0]);
            }
            case syntax::ast_node_types_t::if_expression:
            case syntax::ast_node_types_t::elseif_expression: {
                auto predicate = evaluate(r, node->lhs);
                auto true_branch = evaluate(r, node->children[0]);
                auto false_branch = evaluate(r, node->rhs);
                return make_if(current_scope(), predicate, true_branch, false_branch);
            }
            case syntax::ast_node_types_t::unary_operator: {
                auto it = s_unary_operators.find(node->token.type);
                if (it == s_unary_operators.end())
                    return nullptr;
                return make_unary_operator(
                    current_scope(),
                    it->second,
                    evaluate(r, node->rhs));
            }
            case syntax::ast_node_types_t::binary_operator: {
                auto it = s_binary_operators.find(node->token.type);
                if (it == s_binary_operators.end())
                    return nullptr;
                return make_binary_operator(
                    current_scope(),
                    it->second,
                    evaluate(r, node->lhs),
                    evaluate(r, node->rhs));
            }
            case syntax::ast_node_types_t::proc_call: {
                auto proc_identifier = find_identifier(node->lhs);
                if (proc_identifier != nullptr) {
                    argument_list* args = nullptr;
                    auto expr = evaluate(r, node->rhs);
                    if (expr != nullptr) {
                        args = dynamic_cast<argument_list*>(expr);
                    }
                    return make_procedure_call(
                        current_scope(),
                        proc_identifier,
                        args);
                }
                return nullptr;
            }
            case syntax::ast_node_types_t::argument_list: {
                auto args = make_argument_list(current_scope());
                for (const auto& arg : node->children) {
                    args->add(evaluate(r, arg));
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
                                fmt::format("_{}", count++),
                                nullptr);
                            return_identifier->type(find_type_up(type_node->children[0]->token.value));
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
                            const auto& symbol_node = param_node->lhs->children[0];
                            auto param_identifier = add_identifier_to_scope(
                                r,
                                symbol_node,
                                param_node->rhs,
                                block_scope);
                            auto field = make_field(block_scope, param_identifier);
                            proc_type->parameters().add(field);
                            break;
                        }
                        case syntax::ast_node_types_t::symbol: {
                            auto param_identifier = add_identifier_to_scope(
                                r,
                                param_node,
                                nullptr,
                                block_scope);
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
                auto enum_type = make_enum_type(r, active_scope);
                active_scope->types().add(enum_type);
                add_composite_type_fields(r, enum_type, node->rhs);
                if (!enum_type->initialize(r, this))
                    return nullptr;
                return enum_type;
            }
            case syntax::ast_node_types_t::cast_expression: {
                auto type = find_type_up(node->lhs->token.value);
                if (type == nullptr) {
                    r.add_message(
                        "P002",
                        fmt::format("unknown type '{}'.", node->lhs->token.value),
                        true);
                }
                return make_cast(current_scope(), type, evaluate(r, node->rhs));
            }
            case syntax::ast_node_types_t::alias_expression: {
                return make_alias(current_scope(), evaluate(r, node->lhs));
            }
            case syntax::ast_node_types_t::union_expression: {
                auto union_type = make_union_type(r, current_scope());
                current_scope()->types().add(union_type);
                add_composite_type_fields(r, union_type, node->rhs);
                if (!union_type->initialize(r, this))
                    return nullptr;
                return union_type;
            }
            case syntax::ast_node_types_t::struct_expression: {
                auto struct_type = make_struct_type(r, current_scope());
                current_scope()->types().add(struct_type);
                add_composite_type_fields(r, struct_type, node->rhs);
                if (!struct_type->initialize(r, this))
                    return nullptr;
                return struct_type;
            }
            case syntax::ast_node_types_t::return_statement: {
                auto return_element = make_return(current_scope());
                for (const auto& arg_node : node->rhs->children) {
                    return_element->expressions().push_back(evaluate(r, arg_node));
                }
                return return_element;
            }
            case syntax::ast_node_types_t::import_expression: {
                return make_import(current_scope(), evaluate(r, node->lhs));
            }
            case syntax::ast_node_types_t::constant_expression: {
                auto identifier = dynamic_cast<compiler::identifier*>(evaluate(r, node->rhs));
                if (identifier != nullptr)
                    identifier->constant(true);
                return identifier;
            }
            case syntax::ast_node_types_t::namespace_expression: {
                return make_namespace(current_scope(), evaluate(r, node->rhs));
            }
            default: {
                break;
            }
        }

        return nullptr;
    }

    bool program::compile(
            common::result& r,
            const syntax::ast_node_shared_ptr& root) {
        _block = push_new_block();

        initialize_core_types(r);

        if (!compile_module(r, root))
            return false;

        if (!execute_directives(r))
            return false;

        if (!resolve_unknown_identifiers(r))
            return false;

        if (!resolve_unknown_types(r))
            return false;

        if (!build_data_segments(r))
            return false;

        fmt::print("\n");
        auto segments = _assembler.segments();
        for (auto segment : segments) {
            fmt::print(
                "segment: {}, type: {}\n",
                segment->name,
                segment_type_name(segment->type));
            for (auto symbol : segment->symbols())
                fmt::print(
                    "\taddress: {:08x}, symbol: {}, type: {}, size: {}\n",
                    symbol->address,
                    symbol->name,
                    symbol_type_name(symbol->type),
                    symbol->size);
            fmt::print("\n");
        }

        return !r.is_failed();
    }

    vm::terp* program::terp() {
        return _terp;
    }

    bool program::compile_module(
            common::result& r,
            const syntax::ast_node_shared_ptr& root) {
        evaluate(r, root);
        return !r.is_failed();
    }

    compiler::block* program::block() {
        return _block;
    }

    bool program::run(common::result& r) {
        while (!_terp->has_exited())
            if (!_terp->step(r))
                return false;
        return true;
    }

    compiler::block* program::pop_scope() {
        if (_scope_stack.empty())
            return nullptr;
        auto top = _scope_stack.top();
        _scope_stack.pop();
        return top;
    }

    const element_map_t& program::elements() const {
        return _elements;
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

    any_type* program::make_any_type(
            common::result& r,
            compiler::block* parent_scope) {
        auto type = new compiler::any_type(parent_scope);
        if (!type->initialize(r, this))
            return nullptr;

        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    type_info* program::make_type_info_type(
            common::result& r,
            compiler::block* parent_scope) {
        auto type = new compiler::type_info(parent_scope);
        if (!type->initialize(r, this))
            return nullptr;

        _elements.insert(std::make_pair(type->id(), type));
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
        add_type_to_scope(make_string_type(r, parent_scope));

        add_type_to_scope(make_type_info_type(r, parent_scope));
        add_type_to_scope(make_any_type(r, parent_scope));
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
        _elements.insert(std::make_pair(if_element->id(), if_element));
        return if_element;
    }

    comment* program::make_comment(
            compiler::block* parent_scope,
            comment_type_t type,
            const std::string& value) {
        auto comment = new compiler::comment(parent_scope, type, value);
        _elements.insert(std::make_pair(comment->id(), comment));
        return comment;
    }

    directive* program::make_directive(
            compiler::block* parent_scope,
            const std::string& name,
            element* expr) {
        auto directive1 = new compiler::directive(parent_scope, name, expr);
        _elements.insert(std::make_pair(directive1->id(), directive1));
        return directive1;
    }

    attribute* program::make_attribute(
            compiler::block* parent_scope,
            const std::string& name,
            element* expr) {
        auto attr = new attribute(parent_scope, name, expr);
        _elements.insert(std::make_pair(attr->id(), attr));
        return attr;
    }

    identifier* program::make_identifier(
            compiler::block* parent_scope,
            const std::string& name,
            initializer* expr) {
        auto identifier = new compiler::identifier(
            parent_scope,
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
                        proc_type->scope(),
                        child_node->token.value,
                        evaluate(r, child_node->lhs)));
                    break;
                }
                case syntax::ast_node_types_t::basic_block: {
                    auto basic_block = dynamic_cast<compiler::block*>(evaluate(
                        r,
                        child_node,
                        element_type_t::proc_instance_block));
                    proc_type->instances().push_back(make_procedure_instance(
                        proc_type->scope(),
                        proc_type,
                        basic_block));
                }
                default:
                    break;
            }
        }
    }

    composite_type* program::make_enum_type(
            common::result& r,
            compiler::block* parent_scope) {
        auto type = new compiler::composite_type(
            parent_scope,
            composite_types_t::enum_type,
            fmt::format("__enum_{}__", common::id_pool::instance()->allocate()));
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    initializer* program::make_initializer(
            compiler::block* parent_scope,
            element* expr) {
        auto initializer = new compiler::initializer(
            parent_scope,
            expr);
        _elements.insert(std::make_pair(initializer->id(), initializer));
        return initializer;
    }

    return_element* program::make_return(compiler::block* parent_scope) {
        auto return_element = new compiler::return_element(parent_scope);
        _elements.insert(std::make_pair(return_element->id(), return_element));
        return return_element;
    }

    numeric_type* program::make_numeric_type(
            common::result& r,
            compiler::block* parent_scope,
            const std::string& name,
            int64_t min,
            uint64_t max) {
        auto type = new compiler::numeric_type(parent_scope, name, min, max);
        if (!type->initialize(r, this))
            return nullptr;

        _elements.insert(std::make_pair(type->id(), type));
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

    string_type* program::make_string_type(
            common::result& r,
            compiler::block* parent_scope) {
        auto type = new compiler::string_type(parent_scope);
        if (!type->initialize(r, this))
            return nullptr;

        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    alias* program::make_alias(compiler::block* parent_scope, element* expr) {
        auto alias_type = new compiler::alias(parent_scope, expr);
        _elements.insert(std::make_pair(alias_type->id(), alias_type));
        return alias_type;
    }

    namespace_element* program::make_namespace(
            compiler::block* parent_scope,
            element* expr) {
        auto ns = new compiler::namespace_element(
            parent_scope,
            expr);
        _elements.insert(std::make_pair(ns->id(), ns));
        return ns;
    }

    composite_type* program::make_union_type(
            common::result& r,
            compiler::block* parent_scope) {
        auto type = new compiler::composite_type(
            parent_scope,
            composite_types_t::union_type,
            fmt::format("__union_{}__", common::id_pool::instance()->allocate()));
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    composite_type* program::make_struct_type(
            common::result& r,
            compiler::block* parent_scope) {
        auto type = new compiler::composite_type(
            parent_scope,
            composite_types_t::struct_type,
            fmt::format("__struct_{}__", common::id_pool::instance()->allocate()));
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    procedure_call* program::make_procedure_call(
            compiler::block* parent_scope,
            compiler::identifier* identifier,
            compiler::argument_list* args) {
        auto proc_call = new compiler::procedure_call(
            parent_scope,
            identifier,
            args);
        _elements.insert(std::make_pair(proc_call->id(), proc_call));
        return proc_call;
    }

    argument_list* program::make_argument_list(compiler::block* parent_scope) {
        auto list = new compiler::argument_list(parent_scope);
        _elements.insert(std::make_pair(list->id(), list));
        return list;
    }

    unary_operator* program::make_unary_operator(
            compiler::block* parent_scope,
            operator_type_t type,
            element* rhs) {
        auto unary_operator = new compiler::unary_operator(parent_scope, type, rhs);
        _elements.insert(std::make_pair(unary_operator->id(), unary_operator));
        return unary_operator;
    }

    binary_operator* program::make_binary_operator(
            compiler::block* parent_scope,
            operator_type_t type,
            element* lhs,
            element* rhs) {
        auto binary_operator = new compiler::binary_operator(parent_scope, type, lhs, rhs);
        _elements.insert(std::make_pair(binary_operator->id(), binary_operator));
        return binary_operator;
    }

    void program::remove_element(common::id_t id) {
        auto item = find_element(id);
        if (item == nullptr)
            return;
        _elements.erase(id);
        delete item;
    }

    element* program::find_element(common::id_t id) {
        auto it = _elements.find(id);
        if (it != _elements.end())
            return it->second;
        return nullptr;
    }

    label* program::make_label(
            compiler::block* parent_scope,
            const std::string& name) {
        auto label = new compiler::label(parent_scope, name);
        _elements.insert(std::make_pair(label->id(), label));
        return label;
    }

    field* program::make_field(
            compiler::block* parent_scope,
            compiler::identifier* identifier) {
        auto field = new compiler::field(parent_scope, identifier);
        _elements.insert(std::make_pair(field->id(), field));
        return field;
    }

    float_literal* program::make_float(
            compiler::block* parent_scope,
            double value) {
        auto literal = new compiler::float_literal(parent_scope, value);
        _elements.insert(std::make_pair(literal->id(), literal));
        return literal;
    }

    boolean_literal* program::make_bool(
            compiler::block* parent_scope,
            bool value) {
        auto boolean_literal = new compiler::boolean_literal(parent_scope, value);
        _elements.insert(std::make_pair(boolean_literal->id(), boolean_literal));
        return boolean_literal;
    }

    expression* program::make_expression(
            compiler::block* parent_scope,
            element* expr) {
        auto expression = new compiler::expression(parent_scope, expr);
        _elements.insert(std::make_pair(expression->id(), expression));
        return expression;
    }

    integer_literal* program::make_integer(
            compiler::block* parent_scope,
            uint64_t value) {
        auto literal = new compiler::integer_literal(parent_scope, value);
        _elements.insert(std::make_pair(literal->id(), literal));
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
        _elements.insert(std::make_pair(instance->id(), instance));
        return instance;
    }

    compiler::identifier* program::add_identifier_to_scope(
            common::result& r,
            const syntax::ast_node_shared_ptr& symbol,
            const syntax::ast_node_shared_ptr& rhs,
            compiler::block* parent_scope) {
        auto namespace_type = find_type_up("namespace");

        auto type_find_result = find_identifier_type(r, symbol);

        auto scope = symbol->is_qualified_symbol()
            ? block()
            : parent_scope != nullptr ? parent_scope : current_scope();

        for (size_t i = 0; i < symbol->children.size() - 1; i++) {
            const auto& symbol_node = symbol->children[i];
            auto var = scope->identifiers().find(symbol_node->token.value);
            if (var == nullptr) {
                auto new_scope = make_block(scope, element_type_t::block);
                auto ns_identifier = make_identifier(
                    new_scope,
                    symbol_node->token.value,
                    make_initializer(
                        new_scope,
                        make_namespace(new_scope, new_scope)));
                ns_identifier->type(namespace_type);
                ns_identifier->inferred_type(true);
                scope->identifiers().add(ns_identifier);
                scope = new_scope;
            } else {
                auto expr = var->initializer()->expression();
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

        const auto& final_symbol = symbol->children.back();

        compiler::initializer* init = nullptr;
        if (rhs != nullptr) {
            auto init_expression = evaluate(r, rhs);
            if (init_expression != nullptr)
                init = make_initializer(scope, init_expression);
        }

        auto new_identifier = make_identifier(
            scope,
            final_symbol->token.value,
            init);

        if (type_find_result.type == nullptr && init != nullptr) {
            type_find_result.type = init->expression()->infer_type(this);
            new_identifier->inferred_type(type_find_result.type != nullptr);
        }

        new_identifier->type(type_find_result.type);
        if (type_find_result.type == nullptr) {
            new_identifier->type(make_unknown_type_from_find_result(
                r,
                scope,
                new_identifier,
                type_find_result));
        }

        new_identifier->constant(symbol->is_constant_expression());
        scope->identifiers().add(new_identifier);

        if (init != nullptr
        &&  init->expression()->element_type() == element_type_t::proc_type) {
            add_procedure_instance(
                r,
                dynamic_cast<procedure_type*>(init->expression()),
                rhs);
        }

        return new_identifier;
    }

    cast* program::make_cast(
            compiler::block* parent_scope,
            compiler::type* type,
            element* expr) {
        auto cast = new compiler::cast(parent_scope, type, expr);
        _elements.insert(std::make_pair(cast->id(), cast));
        return cast;
    }

    void program::apply_attributes(
            common::result& r,
            compiler::element* element,
            const syntax::ast_node_shared_ptr& node) {
        for (auto it = node->children.begin();
             it != node->children.end();
             ++it) {
            const auto& child_node = *it;
            if (child_node->type == syntax::ast_node_types_t::attribute) {
                element->attributes().add(dynamic_cast<attribute*>(evaluate(r, child_node)));
            }
        }
    }

    statement* program::make_statement(
            compiler::block* parent_scope,
            label_list_t labels,
            element* expr) {
        auto statement = new compiler::statement(parent_scope, expr);
        for (auto label : labels)
            statement->labels().push_back(label);
        _elements.insert(std::make_pair(statement->id(), statement));
        return statement;
    }

    string_literal* program::make_string(
            compiler::block* parent_scope,
            const std::string& value) {
        auto literal = new compiler::string_literal(parent_scope, value);
        _elements.insert(std::make_pair(literal->id(), literal));
        return literal;
    }

    array_type* program::make_array_type(
            common::result& r,
            compiler::block* parent_scope,
            compiler::type* entry_type,
            size_t size) {
        auto type = new compiler::array_type(
            parent_scope,
            fmt::format("__array_{}_{}__", entry_type->name(), size),
            entry_type);
        if (!type->initialize(r, this))
            return nullptr;
        type->size(size);
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    compiler::block* program::make_block(
            compiler::block* parent_scope,
            element_type_t type) {
        auto block_element = new compiler::block(parent_scope, type);
        _elements.insert(std::make_pair(block_element->id(), block_element));
        return block_element;
    }

    void program::add_composite_type_fields(
            common::result& r,
            compiler::composite_type* type,
            const syntax::ast_node_shared_ptr& block) {
        auto u32_type = find_type_up("u32");

        for (const auto& child : block->children) {
            if (child->type != syntax::ast_node_types_t::statement) {
                // XXX: this is an error!
                break;
            }
            auto expr_node = child->rhs;
            switch (expr_node->type) {
                case syntax::ast_node_types_t::assignment: {
                    auto symbol_node = expr_node->lhs->children[0];
                    auto type_find_result = find_identifier_type(r, symbol_node);
                    auto init = make_initializer(
                        current_scope(),
                        evaluate(r, expr_node->rhs));
                    auto field_identifier = make_identifier(
                        current_scope(),
                        symbol_node->children[0]->token.value,
                        init);
                    if (type_find_result.type == nullptr) {
                        type_find_result.type = init->expression()->infer_type(this);
                        field_identifier->inferred_type(type_find_result.type != nullptr);
                    }
                    field_identifier->type(type_find_result.type);
                    type->fields().add(make_field(
                        current_scope(),
                        field_identifier));
                    break;
                }
                case syntax::ast_node_types_t::symbol: {
                    auto type_find_result = find_identifier_type(r, expr_node);
                    auto field_identifier = make_identifier(
                        current_scope(),
                        expr_node->children[0]->token.value,
                        nullptr);
                    if (type_find_result.type == nullptr) {
                        if (type->type() == composite_types_t::enum_type) {
                            field_identifier->type(u32_type);
                        } else {
                            field_identifier->type(make_unknown_type_from_find_result(
                                r,
                                current_scope(),
                                field_identifier,
                                type_find_result));
                        }
                    } else {
                        field_identifier->type(type_find_result.type);
                    }
                    type->fields().add(make_field(
                        current_scope(),
                        field_identifier));
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
            const std::string& name,
            bool is_array,
            size_t array_size) {
        auto type = new compiler::unknown_type(
            parent_scope,
            name);
        if (!type->initialize(r, this))
            return nullptr;
        type->is_array(is_array);
        type->array_size(array_size);
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    procedure_type* program::make_procedure_type(
            compiler::block* parent_scope,
            compiler::block* block_scope) {
        auto type = new compiler::procedure_type(
            parent_scope,
            block_scope,
            fmt::format("__proc_{}__", common::id_pool::instance()->allocate()));
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    bool program::execute_directives(common::result& r) {
        std::function<bool (compiler::block*)> recursive_execute =
            [&](compiler::block* scope) -> bool {
                for (auto stmt : scope->statements()) {
                    if (stmt->expression()->element_type() == element_type_t::directive) {
                        auto directive_element = dynamic_cast<compiler::directive*>(stmt->expression());
                        if (!directive_element->execute(r, this))
                            return false;
                    }
                }
                for (auto block : scope->blocks()) {
                    if (!recursive_execute(block))
                        return false;
                }
                return true;
            };
        return recursive_execute(block());
    }

    bool program::build_data_segments(common::result& r) {
        std::function<bool (compiler::block*)> recursive_execute =
            [&](compiler::block* scope) -> bool {
                if (scope->element_type() == element_type_t::proc_type_block
                ||  scope->element_type() == element_type_t::proc_instance_block)
                    return true;
                scope->define_data(r, _assembler);
                for (auto block : scope->blocks()) {
                    if (!recursive_execute(block))
                        return false;
                }
                return true;
            };
        return recursive_execute(block());
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
                identifier_type = find_type_down(unknown_type->name());
                if (unknown_type->is_array()) {
                    auto array_type = find_array_type(
                        identifier_type,
                        unknown_type->array_size());
                    if (array_type == nullptr) {
                        array_type = make_array_type(
                            r,
                            dynamic_cast<compiler::block*>(var->parent()),
                            identifier_type,
                            unknown_type->array_size());
                    }
                    identifier_type = array_type;
                }

                if (identifier_type != nullptr) {
                    var->type(identifier_type);
                    remove_element(unknown_type->id());
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
                    fmt::format("unable to resolve type for identifier: {}", var->name()),
                    true);
            }
        }

        return _identifiers_with_unknown_types.empty();
    }

    bool program::resolve_unknown_identifiers(common::result& r) {
        return true;
    }

    compiler::type* program::find_type_down(const std::string& name) {
        std::function<compiler::type* (compiler::block*)> recursive_find =
            [&](compiler::block* scope) -> compiler::type* {
                auto matching_type = scope->types().find(name);
                if (matching_type != nullptr)
                    return matching_type;
                auto type_identifier = scope->identifiers().find(name);
                if (type_identifier != nullptr)
                    return type_identifier->type();
                for (auto block : scope->blocks()) {
                    auto* type = recursive_find(block);
                    if (type != nullptr)
                        return type;
                }

                return nullptr;
            };
        return recursive_find(block());
    }

    compiler::type* program::find_type_up(const std::string& name) const {
        auto scope = current_scope();
        while (scope != nullptr) {
            auto type = scope->types().find(name);
            if (type != nullptr)
                return type;
            auto type_identifier = scope->identifiers().find(name);
            if (type_identifier != nullptr)
                return type_identifier->type();
            scope = dynamic_cast<compiler::block*>(scope->parent());
        }
        return nullptr;
    }

    namespace_type* program::make_namespace_type(
            common::result& r,
            compiler::block* parent_scope) {
        auto type = new compiler::namespace_type(parent_scope);
        if (!type->initialize(r, this))
            return nullptr;

        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    type_find_result_t program::find_identifier_type(
            common::result& r,
            const syntax::ast_node_shared_ptr& symbol) {
        type_find_result_t result {};

        if (symbol->rhs != nullptr) {
            result.type_name = symbol->rhs->token.value;
            result.is_array = symbol->rhs->is_array();
            result.array_size = 0; // XXX: this needs to be fixed!

            if (!result.type_name.empty()) {
                result.type = find_type_up(result.type_name);
                if (result.is_array) {
                    auto array_type = find_array_type(
                        result.type,
                        result.array_size);
                    if (array_type == nullptr) {
                        array_type = make_array_type(
                            r,
                            current_scope(),
                            result.type,
                            result.array_size);
                    }
                    result.type = array_type;
                }
            }
        }

        return result;
    }

    unknown_type* program::make_unknown_type_from_find_result(
            common::result& r,
            compiler::block* scope,
            compiler::identifier* identifier,
            const type_find_result_t& result) {
        auto unknown_type = make_unknown_type(
            r,
            scope,
            result.type_name,
            result.is_array,
            result.array_size);
        _identifiers_with_unknown_types.push_back(identifier);
        return unknown_type;
    }

    import* program::make_import(compiler::block* parent_scope, element* expr) {
        auto import_element = new compiler::import(parent_scope, expr);
        _elements.insert(std::make_pair(import_element->id(), import_element));
        return import_element;
    }

    compiler::type* program::find_array_type(compiler::type* entry_type, size_t size) {
        return find_type_up(fmt::format("__array_{}_{}__", entry_type->name(), size));
    }

    compiler::identifier* program::find_identifier(const syntax::ast_node_shared_ptr& node) {
        auto var = (identifier*) nullptr;
        if (node->is_qualified_symbol()) {
            auto block_scope = block();
            for (const auto& symbol_node : node->children) {
                var = block_scope->identifiers().find(symbol_node->token.value);
                if (var == nullptr || var->initializer() == nullptr)
                    return nullptr;
                auto expr = var->initializer()->expression();
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
                var = block_scope->identifiers().find(symbol_part->token.value);
                if (var != nullptr)
                    return var;
                block_scope = dynamic_cast<compiler::block*>(block_scope->parent());
            }
            return nullptr;
        }
        return var;
    }

    void program::add_expression_to_scope(compiler::block* scope, compiler::element* expr) {
        switch (expr->element_type()) {
            case element_type_t::comment:
                scope->comments().push_back(dynamic_cast<comment*>(expr));
                break;
            case element_type_t::attribute:
                scope->attributes().add(dynamic_cast<attribute*>(expr));
                break;
            case element_type_t::statement: {
                scope->statements().push_back(dynamic_cast<statement*>(expr));
                break;
            }
            default:
                break;
        }
    }

};