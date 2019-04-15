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
#include <common/id_pool.h>
#include "ast.h"

namespace basecode::syntax {

    ast_node_data_t* ast_node_t::get_data(ast_builder* builder) {
        ast_node_data_t* data = nullptr;
        if (data_id == 0) {
            data = builder->make_node_data();
            data_id = data->id;
        } else {
            data = builder->find_data(data_id);
        }
        return data;
    }

    bool ast_node_t::has_attribute(ast_builder* builder, const std::string_view& name) const {
        if (!has_data())
            return false;
        auto data = builder->find_data(data_id);
        if (data == nullptr)
            return false;
        if (data->attributes.empty())
            return false;
        for (auto attr : data->attributes)
            if (attr->token != nullptr && attr->token->value == name)
                return true;
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_builder::ast_builder() : _node_storage(256),
                                 _data_storage(256) {
    }

    void ast_builder::reset() {
        while (!_with_stack.empty())
            _with_stack.pop();
        while (!_case_stack.empty())
            _case_stack.pop();
        while (!_switch_stack.empty())
            _switch_stack.pop();
        while (!_scope_stack.empty())
            _scope_stack.pop();
        while (!_member_access_stack.empty())
            _member_access_stack.pop();
    }

    //
    ast_node_t* ast_builder::find_node(common::id_t id) {
        auto it = _nodes.find(id);
        if (it == std::end(_nodes))
            return nullptr;
        return it->second;
    }

    ast_node_data_t* ast_builder::find_data(common::id_t id) {
        auto it = _datas.find(id);
        if (it == std::end(_datas))
            return nullptr;
        return it->second;
    }

    //
    ast_node_t* ast_builder::clone(const ast_node_t* other) {
        if (other == nullptr)
            return nullptr;

        auto node = make_node(other->type, other->token);
        node->lhs = clone(other->lhs);
        node->rhs = clone(other->rhs);
        node->location = other->location;
        for (auto child : other->children)
            node->children.emplace_back(clone(child));

        auto data = clone_data(other->data_id);
        if (data != nullptr)
            node->data_id = data->id;

        return node;
    }

    ast_node_data_t* ast_builder::clone_data(common::id_t id) {
        if (id == 0)
            return nullptr;

        auto other = find_data(id);
        if (other == nullptr)
            return nullptr;

        auto data = make_node_data();
        data->is_uniform_function_call = other->is_uniform_function_call;
        for (auto label : other->labels)
            data->labels.emplace_back(clone(label));
        for (auto comment : other->comments)
            data->comments.emplace_back(clone(comment));
        for (auto attr : other->attributes)
            data->attributes.emplace_back(clone(attr));
        return data;
    }

    // with stack
    ast_node_t* ast_builder::pop_with() {
        if (_with_stack.empty())
            return nullptr;
        auto top = _with_stack.top();
        _with_stack.pop();
        return top;
    }

    ast_node_t* ast_builder::current_with() const {
        if (_with_stack.empty())
            return nullptr;
        return _with_stack.top();
    }

    void ast_builder::push_with(ast_node_t* node) {
        _with_stack.push(node);
    }

    // case stack
    ast_node_t* ast_builder::pop_case() {
        if (_case_stack.empty())
            return nullptr;
        auto top = _case_stack.top();
        _case_stack.pop();
        return top;
    }

    ast_node_t* ast_builder::current_case() const {
        if (_case_stack.empty())
            return nullptr;
        return _case_stack.top();
    }

    void ast_builder::push_case(ast_node_t* node) {
        _case_stack.push(node);
    }

    // switch stack
    ast_node_t* ast_builder::pop_switch() {
        if (_switch_stack.empty())
            return nullptr;
        auto top = _switch_stack.top();
        _switch_stack.pop();
        return top;
    }

    ast_node_t* ast_builder::current_switch() const {
        if (_switch_stack.empty())
            return nullptr;
        return _switch_stack.top();
    }

    void ast_builder::push_switch(ast_node_t* node) {
        _switch_stack.push(node);
    }

    // member access stack
    ast_node_t* ast_builder::pop_member_access() {
        if (_member_access_stack.empty())
            return nullptr;
        auto top = _member_access_stack.top();
        _member_access_stack.pop();
        return top;
    }

    ast_node_t* ast_builder::current_member_access() const {
        if (_member_access_stack.empty())
            return nullptr;
        return _member_access_stack.top();
    }

    void ast_builder::push_member_access(ast_node_t* node) {
        _member_access_stack.push(node);
    }

    //
    ast_node_t* ast_builder::pair_node() {
        return make_node(ast_node_type_t::pair);
    }

    ast_node_t* ast_builder::pop_scope() {
        if (_scope_stack.empty())
            return nullptr;
        auto top = _scope_stack.top();
        _scope_stack.pop();
        return top;
    }

    ast_node_t* ast_builder::end_scope() {
        return pop_scope();
    }

    ast_node_t* ast_builder::current_scope() const {
        if (_scope_stack.empty())
            return nullptr;
        return _scope_stack.top();
    }

    ast_node_t* ast_builder::symbol_node() {
        auto node = make_node(ast_node_type_t::symbol);
        node->lhs = type_list_node();
        return node;
    }

    ast_node_t* ast_builder::module_node() {
        auto node = make_node(ast_node_type_t::module);
        push_scope(node);
        return node;
    }

    ast_node_t* ast_builder::begin_scope() {
        if (_scope_stack.empty()) {
            return module_node();
        } else {
            return basic_block_node();
        }
    }

    ast_node_t* ast_builder::type_list_node() {
        return make_node(ast_node_type_t::type_list);
    }

    ast_node_t* ast_builder::proc_call_node() {
        auto node = make_node(ast_node_type_t::proc_call);
        node->lhs = proc_call_binding_node();
        node->rhs = argument_list_node();
        return node;
    }

    ast_node_t* ast_builder::statement_node() {
        return make_node(ast_node_type_t::statement);
    }

    ast_node_t* ast_builder::expression_node() {
        return make_node(ast_node_type_t::expression);
    }

    ast_node_t* ast_builder::proc_types_node() {
        auto node = make_node(ast_node_type_t::proc_types);
        node->lhs = type_parameter_list_node();
        node->rhs = parameter_list_node();
        return node;
    }

    ast_node_t* ast_builder::assignment_node() {
        auto node = make_node(ast_node_type_t::assignment);
        node->lhs = assignment_target_list_node();
        node->rhs = assignment_source_list_node();
        return node;
    }

    ast_node_t* ast_builder::basic_block_node() {
        auto node = make_node(ast_node_type_t::basic_block);
        push_scope(node);
        return node;
    }

    ast_node_t* ast_builder::binary_operator_node(
            ast_node_t* lhs,
            const token_t* token,
            ast_node_t* rhs) {
        auto node = make_node(ast_node_type_t::binary_operator, token);
        node->lhs = lhs;
        node->rhs = rhs;
        node->location.end(rhs->location.end());
        node->location.start(lhs->location.start());
        return node;
    }

    ast_node_t* ast_builder::argument_list_node() {
        return make_node(ast_node_type_t::argument_list);
    }

    ast_node_t* ast_builder::assignment_set_node() {
        auto node = make_node(ast_node_type_t::assignment_set);
        node->lhs = assignment_target_list_node();
        node->rhs = assignment_source_list_node();
        return node;
    }

    ast_node_t* ast_builder::statement_body_node() {
        return make_node(ast_node_type_t::statement_body);
    }

    ast_node_t* ast_builder::parameter_list_node() {
        return make_node(ast_node_type_t::parameter_list);
    }

    ast_node_t* ast_builder::type_parameter_node() {
        return make_node(ast_node_type_t::type_parameter);
    }

    ast_node_t* ast_builder::type_declaration_node() {
        return make_node(ast_node_type_t::type_declaration);
    }

    ast_node_t* ast_builder::proc_call_binding_node() {
        auto node = make_node(ast_node_type_t::proc_call_binding);
        node->lhs = type_list_node();
        return node;
    }

    ast_node_t* ast_builder::with_member_access_node() {
        return make_node(ast_node_type_t::with_member_access);
    }

    ast_node_t* ast_builder::subscript_operator_node() {
        return make_node(ast_node_type_t::subscript_operator);
    }

    ast_node_t* ast_builder::type_tagged_symbol_node() {
        auto node = make_node(ast_node_type_t::type_tagged_symbol);
        node->lhs = type_list_node();
        return node;
    }

    ast_node_t* ast_builder::pointer_declaration_node() {
        return make_node(ast_node_type_t::pointer_declaration);
    }

    ast_node_t* ast_builder::constant_assignment_node() {
        auto node = make_node(ast_node_type_t::constant_assignment);
        node->lhs = assignment_target_list_node();
        node->rhs = assignment_source_list_node();
        return node;
    }

    ast_node_t* ast_builder::type_parameter_list_node() {
        return make_node(ast_node_type_t::type_parameter_list);
    }

    ast_node_t* ast_builder::array_subscript_list_node() {
        return make_node(ast_node_type_t::array_subscript_list);
    }

    ast_node_t* ast_builder::return_argument_list_node() {
        return make_node(ast_node_type_t::return_argument_list);
    }

    ast_node_t* ast_builder::subscript_declaration_node() {
        return make_node(ast_node_type_t::subscript_declaration);
    }

    void ast_builder::push_scope(ast_node_t* node) {
        _scope_stack.push(node);
    }

    ast_node_t* ast_builder::assignment_source_list_node() {
        return make_node(ast_node_type_t::assignment_source_list);
    }

    ast_node_t* ast_builder::assignment_target_list_node() {
        return make_node(ast_node_type_t::assignment_target_list);
    }

    ast_node_t* ast_builder::if_node(const token_t* token) {
        return make_node(ast_node_type_t::if_expression, token);
    }

    ast_node_t* ast_builder::case_node(const token_t* token) {
        return make_node(ast_node_type_t::case_expression, token);
    }

    ast_node_t* ast_builder::with_node(const token_t* token) {
        return make_node(ast_node_type_t::with_expression, token);
    }

    ast_node_t* ast_builder::else_node(const token_t* token) {
        return make_node(ast_node_type_t::else_expression, token);
    }

    ast_node_t* ast_builder::from_node(const token_t* token) {
        return make_node(ast_node_type_t::from_expression, token);
    }

    ast_node_t* ast_builder::enum_node(const token_t* token) {
        auto node = make_node(ast_node_type_t::enum_expression, token);
        node->lhs = type_parameter_list_node();
        return node;
    }

    ast_node_t* ast_builder::cast_node(const token_t* token) {
        auto node = make_node(ast_node_type_t::cast_expression, token);
        node->lhs = type_parameter_list_node();
        node->rhs = argument_list_node();
        return node;
    }

    ast_node_t* ast_builder::family_node(const token_t* token) {
        auto node = make_node(ast_node_type_t::family_expression, token);
        node->lhs = type_parameter_list_node();
        node->rhs = argument_list_node();
        return node;
    }

    ast_node_t* ast_builder::while_node(const token_t* token) {
        return make_node(ast_node_type_t::while_statement, token);
    }

    ast_node_t* ast_builder::union_node(const token_t* token) {
        auto node = make_node(ast_node_type_t::union_expression, token);
        node->lhs = type_parameter_list_node();
        return node;
    }

    ast_node_t* ast_builder::yield_node(const token_t* token) {
        return make_node(ast_node_type_t::yield_expression, token);
    }

    ast_node_t* ast_builder::defer_node(const token_t* token) {
        return make_node(ast_node_type_t::defer_expression, token);
    }

    ast_node_t* ast_builder::break_node(const token_t* token) {
        return make_node(ast_node_type_t::break_statement, token);
    }

    ast_node_t* ast_builder::label_node(const token_t* token) {
        return make_node(ast_node_type_t::label, token);
    }

    ast_node_t* ast_builder::import_node(const token_t* token) {
        return make_node(ast_node_type_t::import_expression, token);
    }

    ast_node_t* ast_builder::return_node(const token_t* token) {
        auto node = make_node(ast_node_type_t::return_statement, token);
        node->rhs = return_argument_list_node();
        return node;
    }

    ast_node_t* ast_builder::for_in_node(const token_t* token) {
        return make_node(ast_node_type_t::for_in_statement, token);
    }

    ast_node_t* ast_builder::struct_node(const token_t* token) {
        auto node = make_node(ast_node_type_t::struct_expression, token);
        node->lhs = type_parameter_list_node();
        return node;
    }

    ast_node_t* ast_builder::switch_node(const token_t* token) {
        return make_node(ast_node_type_t::switch_expression, token);
    }

    ast_node_t* ast_builder::else_if_node(const token_t* token) {
        return make_node(ast_node_type_t::elseif_expression, token);
    }

    ast_node_t* ast_builder::continue_node(const token_t* token) {
        return make_node(ast_node_type_t::continue_statement, token);
    }

    ast_node_t* ast_builder::raw_block_node(const token_t* token) {
        return make_node(ast_node_type_t::raw_block, token);
    }

    ast_node_t* ast_builder::transmute_node(const token_t* token) {
        auto node = make_node(ast_node_type_t::transmute_expression, token);
        node->lhs = type_parameter_list_node();
        node->rhs = argument_list_node();
        return node;
    }

    ast_node_t* ast_builder::namespace_node(const token_t* token) {
        return make_node(ast_node_type_t::namespace_expression, token);
    }

    ast_node_t* ast_builder::directive_node(const token_t* token) {
        return make_node(ast_node_type_t::directive, token);
    }

    ast_node_t* ast_builder::attribute_node(const token_t* token) {
        return make_node(ast_node_type_t::attribute, token);
    }

    ast_node_t* ast_builder::new_literal_node(const token_t* token) {
        auto node = make_node(ast_node_type_t::new_literal, token);
        node->lhs = type_parameter_list_node();
        node->rhs = argument_list_node();
        return node;
    }

    ast_node_t* ast_builder::fallthrough_node(const token_t* token) {
        return make_node(ast_node_type_t::fallthrough_statement, token);
    }

    ast_node_t* ast_builder::symbol_part_node(const token_t* token) {
        return make_node(ast_node_type_t::symbol_part, token);
    }

    ast_node_t* ast_builder::nil_literal_node(const token_t* token) {
        return make_node(ast_node_type_t::nil_literal, token);
    }

    ast_node_t* ast_builder::line_comment_node(const token_t* token) {
        return make_node(ast_node_type_t::line_comment, token);
    }

    ast_node_t* ast_builder::block_comment_node(const token_t* token) {
        return make_node(ast_node_type_t::block_comment, token);
    }

    ast_node_t* ast_builder::array_literal_node(const token_t* token) {
        auto node = make_node(ast_node_type_t::array_literal, token);
        node->lhs = type_parameter_list_node();
        node->rhs = argument_list_node();
        return node;
    }

    ast_node_t* ast_builder::tuple_literal_node(const token_t* token) {
        auto node = make_node(ast_node_type_t::tuple_literal, token);
        node->lhs = type_parameter_list_node();
        node->rhs = argument_list_node();
        return node;
    }

    ast_node_t* ast_builder::unary_operator_node(const token_t* token) {
        return make_node(ast_node_type_t::unary_operator, token);
    }

    ast_node_t* ast_builder::string_literal_node(const token_t* token) {
        return make_node(ast_node_type_t::string_literal, token);
    }

    ast_node_t* ast_builder::number_literal_node(const token_t* token) {
        return make_node(ast_node_type_t::number_literal, token);
    }

    ast_node_t* ast_builder::proc_expression_node(const token_t* token) {
        auto node = make_node(ast_node_type_t::proc_expression, token);
        node->lhs = proc_types_node();
        node->rhs = parameter_list_node();
        return node;
    }

    ast_node_t* ast_builder::spread_operator_node(const token_t* token) {
        return make_node(ast_node_type_t::spread_operator, token);
    }

    ast_node_t* ast_builder::boolean_literal_node(const token_t* token) {
        return make_node(ast_node_type_t::boolean_literal, token);
    }

    ast_node_t* ast_builder::module_expression_node(const token_t* token) {
        return make_node(ast_node_type_t::module_expression, token);
    }

    ast_node_t* ast_builder::character_literal_node(const token_t* token) {
        return make_node(ast_node_type_t::character_literal, token);
    }

    ast_node_t* ast_builder::lambda_expression_node(const token_t* token) {
        auto node = make_node(ast_node_type_t::lambda_expression, token);
        node->lhs = proc_types_node();
        node->rhs = parameter_list_node();
        return node;
    }

    ast_node_t* ast_builder::value_sink_literal_node(const token_t* token) {
        return make_node(ast_node_type_t::value_sink_literal, token);
    }

    ast_node_t* ast_builder::uninitialized_literal_node(const token_t* token) {
        return make_node(ast_node_type_t::uninitialized_literal, token);
    }

    ast_node_data_t* ast_builder::make_node_data() {
        auto data = _data_storage.alloc();
        data->id = common::id_pool::instance()->allocate();
        _datas.insert(std::make_pair(data->id, data));
        return data;
    }

    ast_node_t* ast_builder::make_node(ast_node_type_t type, const token_t* token) {
        auto node = _node_storage.alloc();
        node->id = common::id_pool::instance()->allocate();
        node->type = type;
        if (token != nullptr) {
            node->token = token;
            node->location = node->token->location;
        }
        _nodes.insert(std::make_pair(node->id, node));
        return node;
    }

}