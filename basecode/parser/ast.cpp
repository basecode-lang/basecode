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

#include <set>
#include <fmt/format.h>
#include "ast.h"

namespace basecode::syntax {

    void ast_builder::reset() {
        _id = 0;
        while (!_with_stack.empty())
            _with_stack.pop();
        while (!_scope_stack.empty())
            _scope_stack.pop();
    }

    // with stack
    ast_node_shared_ptr ast_builder::pop_with() {
        if (_with_stack.empty())
            return nullptr;
        auto top = _with_stack.top();
        _with_stack.pop();
        return top;
    }

    ast_node_shared_ptr ast_builder::current_with() const {
        if (_with_stack.empty())
            return nullptr;
        return _with_stack.top();
    }

    void ast_builder::push_with(const ast_node_shared_ptr& node) {
        _with_stack.push(node);
    }

    //
    void ast_builder::configure_node(
            const ast_node_shared_ptr& node,
            const token_t& token,
            ast_node_types_t type) {
        node->id = ++_id;
        node->type = type;
        node->token = token;
        node->location = token.location;
    }

    ast_node_shared_ptr ast_builder::pair_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::pair;
        return node;
    }

    ast_node_shared_ptr ast_builder::pop_scope() {
        if (_scope_stack.empty())
            return nullptr;
        auto top = _scope_stack.top();
        _scope_stack.pop();
        return top;
    }

    ast_node_shared_ptr ast_builder::end_scope() {
        return pop_scope();
    }

    ast_node_t* ast_builder::current_scope() const {
        if (_scope_stack.empty())
            return nullptr;
        return _scope_stack.top().get();
    }

    ast_node_shared_ptr ast_builder::symbol_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::symbol;
        return node;
    }

    ast_node_shared_ptr ast_builder::module_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::module;
        push_scope(node);
        return node;
    }

    ast_node_shared_ptr ast_builder::begin_scope() {
        if (_scope_stack.empty()) {
            return module_node();
        } else {
            return basic_block_node();
        }
    }

    ast_node_shared_ptr ast_builder::type_list_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::type_list;
        return node;
    }

    ast_node_shared_ptr ast_builder::proc_call_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::proc_call;
        node->rhs = argument_list_node();
        return node;
    }

    ast_node_shared_ptr ast_builder::statement_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::statement;
        return node;
    }

    ast_node_shared_ptr ast_builder::subscript_operator_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::subscript_operator;
        return node;
    }

    ast_node_shared_ptr ast_builder::expression_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::expression;
        return node;
    }

    ast_node_shared_ptr ast_builder::proc_types_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::proc_types;
        node->lhs = type_parameter_list_node();
        node->rhs = type_list_node();
        return node;
    }

    ast_node_shared_ptr ast_builder::assignment_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::assignment;
        node->lhs = assignment_target_list_node();
        node->rhs = assignment_source_list_node();
        return node;
    }

    ast_node_shared_ptr ast_builder::basic_block_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::basic_block;
        push_scope(node);
        return node;
    }

    ast_node_shared_ptr ast_builder::binary_operator_node(
            const ast_node_shared_ptr& lhs,
            const token_t& token,
            const ast_node_shared_ptr& rhs) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::binary_operator);
        node->lhs = lhs;
        node->rhs = rhs;
        node->location.start(lhs->location.start());
        node->location.end(rhs->location.end());
        return node;
    }

    ast_node_shared_ptr ast_builder::argument_list_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::argument_list;
        return node;
    }

    ast_node_shared_ptr ast_builder::statement_body_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::statement_body;
        return node;
    }

    ast_node_shared_ptr ast_builder::parameter_list_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::parameter_list;
        return node;
    }

    ast_node_shared_ptr ast_builder::type_identifier_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::type_identifier;
        return node;
    }

    ast_node_shared_ptr ast_builder::spread_operator_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::spread_operator;
        return node;
    }

    ast_node_shared_ptr ast_builder::new_expression_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::new_expression;
        node->rhs = argument_list_node();
        return node;
    }

    ast_node_shared_ptr ast_builder::map_expression_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::map_expression;
        node->lhs = type_list_node();
        node->rhs = argument_list_node();
        return node;
    }

    ast_node_shared_ptr ast_builder::type_parameter_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::type_parameter;
        return node;
    }

    ast_node_shared_ptr ast_builder::tuple_expression_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::tuple_expression;
        node->rhs = argument_list_node();
        return node;
    }

    ast_node_shared_ptr ast_builder::array_expression_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::array_expression;
        node->rhs = argument_list_node();
        return node;
    }

    ast_node_shared_ptr ast_builder::with_member_access_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::with_member_access;
        return node;
    }

    ast_node_shared_ptr ast_builder::constant_assignment_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::constant_assignment;
        node->lhs = assignment_target_list_node();
        node->rhs = assignment_source_list_node();
        return node;
    }

    ast_node_shared_ptr ast_builder::type_parameter_list_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::type_parameter_list;
        return node;
    }

    ast_node_shared_ptr ast_builder::array_subscript_list_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::array_subscript_list;
        return node;
    }

    ast_node_shared_ptr ast_builder::return_argument_list_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::return_argument_list;
        return node;
    }

    void ast_builder::push_scope(const ast_node_shared_ptr& node) {
        _scope_stack.push(node);
    }

    ast_node_shared_ptr ast_builder::assignment_source_list_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::assignment_source_list;
        return node;
    }

    ast_node_shared_ptr ast_builder::assignment_target_list_node() {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::assignment_target_list;
        return node;
    }

    ast_node_shared_ptr ast_builder::if_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::if_expression);
        return node;
    }

    ast_node_shared_ptr ast_builder::with_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::with_expression);
        return node;
    }

    ast_node_shared_ptr ast_builder::else_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::else_expression);
        return node;
    }

    ast_node_shared_ptr ast_builder::cast_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::cast_expression);
        return node;
    }

    ast_node_shared_ptr ast_builder::from_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::from_expression);
        return node;
    }

    ast_node_shared_ptr ast_builder::enum_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::enum_expression);
        node->lhs = type_parameter_list_node();
        return node;
    }

    ast_node_shared_ptr ast_builder::while_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::while_statement);
        return node;
    }

    ast_node_shared_ptr ast_builder::union_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::union_expression);
        node->lhs = type_parameter_list_node();
        return node;
    }

    ast_node_shared_ptr ast_builder::defer_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::defer_expression);
        return node;
    }

    ast_node_shared_ptr ast_builder::break_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::break_statement);
        return node;
    }

    ast_node_shared_ptr ast_builder::label_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::label);
        return node;
    }

    ast_node_shared_ptr ast_builder::import_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::import_expression);
        return node;
    }

    ast_node_shared_ptr ast_builder::return_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::return_statement;
        node->rhs = return_argument_list_node();
        node->location = token.location;
        return node;
    }

    ast_node_shared_ptr ast_builder::for_in_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::for_in_statement);
        node->location = token.location;
        return node;
    }

    ast_node_shared_ptr ast_builder::struct_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::struct_expression);
        node->lhs = type_parameter_list_node();
        return node;
    }

    ast_node_shared_ptr ast_builder::else_if_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::elseif_expression;
        node->location = token.location;
        return node;
    }

    ast_node_shared_ptr ast_builder::continue_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::continue_statement);
        return node;
    }

    ast_node_shared_ptr ast_builder::raw_block_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::raw_block);
        return node;
    }

    ast_node_shared_ptr ast_builder::namespace_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::namespace_expression);
        return node;
    }

    ast_node_shared_ptr ast_builder::directive_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::directive);
        return node;
    }

    ast_node_shared_ptr ast_builder::attribute_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::attribute);
        return node;
    }

    ast_node_shared_ptr ast_builder::proc_expression_node(token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        node->id = ++_id;
        node->type = ast_node_types_t::proc_expression;
        node->lhs = proc_types_node();
        node->rhs = parameter_list_node();
        node->location = token.location;
        return node;
    }

    ast_node_shared_ptr ast_builder::transmute_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::transmute_expression);
        return node;
    }

    ast_node_shared_ptr ast_builder::symbol_part_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::symbol_part);
        return node;
    }

    ast_node_shared_ptr ast_builder::nil_literal_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::nil_literal);
        return node;
    }

    ast_node_shared_ptr ast_builder::line_comment_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::line_comment);
        return node;
    }

    ast_node_shared_ptr ast_builder::block_comment_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::block_comment);
        return node;
    }

    ast_node_shared_ptr ast_builder::unary_operator_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::unary_operator);
        return node;
    }

    ast_node_shared_ptr ast_builder::string_literal_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::string_literal);
        return node;
    }

    ast_node_shared_ptr ast_builder::number_literal_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::number_literal);
        return node;
    }

    ast_node_shared_ptr ast_builder::boolean_literal_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::boolean_literal);
        return node;
    }

    ast_node_shared_ptr ast_builder::module_expression_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::module_expression);
        return node;
    }

    ast_node_shared_ptr ast_builder::character_literal_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        configure_node(node, token, ast_node_types_t::character_literal);
        return node;
    }

};