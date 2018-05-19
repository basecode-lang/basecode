#include <fmt/format.h>
#include "ast.h"

namespace basecode {

    ///////////////////////////////////////////////////////////////////////////

    ast_formatter::ast_formatter(const ast_node_shared_ptr& root) : _root(root) {
    }

    void ast_formatter::format() {
        format_node(_root, 0);
    }

    void ast_formatter::format_node(
            const ast_node_shared_ptr& node,
            uint32_t level) {
        if (node == nullptr) {
            fmt::print("nullptr");
            return;
        }
        fmt::print(
            "[type: {} | token: {}]\n",
            node->name(),
            node->token.name());
        fmt::print("{1:{0}}     value: '{2}'\n", level, "", node->token.value);
        if (node->token.is_numeric()) {
            fmt::print("{1:{0}}     radix: {2}\n", level, "", node->token.radix);
            switch (node->token.number_type) {
                case number_types_t::none:
                    fmt::print("{1:{0}}      type: none\n", level, "");
                    break;
                case number_types_t::integer:
                    fmt::print("{1:{0}}      type: integer\n", level, "");
                    break;
                case number_types_t::floating_point:
                    fmt::print("{1:{0}}      type: floating_point\n", level, "");
                    break;
            }
        }
        fmt::print("{1:{0}}is_pointer: {2}\n", level, "", node->is_pointer());
        fmt::print("{1:{0}}  is_array: {2}\n", level, "", node->is_array());
        fmt::print("{1:{0}}             --\n", level, "");
        fmt::print("{1:{0}}       lhs: ", level, "");
        format_node(node->lhs, level + 7);
        fmt::print("\n");
        fmt::print("{1:{0}}             --\n", level, "");
        fmt::print("{1:{0}}       rhs: ", level, "");
        format_node(node->rhs, level + 7);
        fmt::print("\n");
        fmt::print("{1:{0}}             --\n", level, "");

        auto index = 0;
        for (auto child : node->children) {
            fmt::print("{1:{0}}      [{2:02}] ", level, "", index++);
            format_node(child, level + 6);
            fmt::print("\n");
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_builder::ast_builder() {
    }

    ast_builder::~ast_builder() {
    }

    ast_node_shared_ptr ast_builder::if_node() {
        auto node = std::make_shared<ast_node_t>();
        node->type = ast_node_types_t::if_expression;
        return node;
    }

    ast_node_shared_ptr ast_builder::else_node() {
        auto node = std::make_shared<ast_node_t>();
        node->type = ast_node_types_t::else_expression;
        return node;
    }

    ast_node_shared_ptr ast_builder::pop_scope() {
        if (_scope_stack.empty())
            return nullptr;
        auto top = _scope_stack.top();
        _scope_stack.pop();
        return top;
    }

    ast_node_t* ast_builder::current_scope() const {
        if (_scope_stack.empty())
            return nullptr;
        return _scope_stack.top().get();
    }

    ast_node_shared_ptr ast_builder::else_if_node() {
        auto node = std::make_shared<ast_node_t>();
        node->type = ast_node_types_t::else_expression;
        return node;
    }

    ast_node_shared_ptr ast_builder::program_node() {
        auto node = std::make_shared<ast_node_t>();
        node->type = ast_node_types_t::program;
        push_scope(node);
        return node;
    }

    ast_node_shared_ptr ast_builder::end_scope() {
        return pop_scope();
    }

    ast_node_shared_ptr ast_builder::begin_scope() {
        if (_scope_stack.empty()) {
            return program_node();
        } else {
            return basic_block_node();
        }
    }

    ast_node_shared_ptr ast_builder::fn_call_node() {
        auto node = std::make_shared<ast_node_t>();
        node->type = ast_node_types_t::fn_call;
        node->rhs = argument_list_node();
        return node;
    }

    ast_node_shared_ptr ast_builder::fn_decl_node() {
        auto node = std::make_shared<ast_node_t>();
        node->type = ast_node_types_t::fn_expression;
        node->rhs = argument_list_node();
        return node;
    }

    ast_node_shared_ptr ast_builder::statement_node() {
        auto node = std::make_shared<ast_node_t>();
        node->type = ast_node_types_t::statement;
        return node;
    }

    ast_node_shared_ptr ast_builder::expression_node() {
        auto node = std::make_shared<ast_node_t>();
        node->type = ast_node_types_t::expression;
        return node;
    }

    ast_node_shared_ptr ast_builder::assignment_node() {
        auto node = std::make_shared<ast_node_t>();
        node->type = ast_node_types_t::assignment;
        return node;
    }

    ast_node_shared_ptr ast_builder::basic_block_node() {
        auto node = std::make_shared<ast_node_t>();
        node->type = ast_node_types_t::basic_block;
        push_scope(node);
        return node;
    }

    ast_node_shared_ptr ast_builder::binary_operator_node(
            const ast_node_shared_ptr& lhs,
            const token_t& token,
            const ast_node_shared_ptr& rhs) {
        auto node = std::make_shared<ast_node_t>();
        node->token = token;
        node->type = ast_node_types_t::binary_operator;
        node->lhs = lhs;
        node->rhs = rhs;
        return node;
    }

    ast_node_shared_ptr ast_builder::argument_list_node() {
        auto node = std::make_shared<ast_node_t>();
        node->type = ast_node_types_t::argument_list;
        return node;
    }

    void ast_builder::push_scope(const ast_node_shared_ptr& node) {
        _scope_stack.push(node);
    }

    ast_node_shared_ptr ast_builder::break_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        node->token = token;
        node->type = ast_node_types_t::break_statement;
        return node;
    }

    ast_node_shared_ptr ast_builder::continue_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        node->token = token;
        node->type = ast_node_types_t::continue_statement;
        return node;
    }

    ast_node_shared_ptr ast_builder::attribute_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        node->token = token;
        node->type = ast_node_types_t::attribute;
        return node;
    }

    ast_node_shared_ptr ast_builder::null_literal_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        node->token = token;
        node->type = ast_node_types_t::null_literal;
        return node;
    }

    ast_node_shared_ptr ast_builder::none_literal_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        node->token = token;
        node->type = ast_node_types_t::none_literal;
        return node;
    }

    ast_node_shared_ptr ast_builder::line_comment_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        node->token = token;
        node->type = ast_node_types_t::line_comment;
        return node;
    }

    ast_node_shared_ptr ast_builder::block_comment_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        node->token = token;
        node->type = ast_node_types_t::block_comment;
        return node;
    }

    ast_node_shared_ptr ast_builder::empty_literal_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        node->token = token;
        node->type = ast_node_types_t::empty_literal;
        return node;
    }

    ast_node_shared_ptr ast_builder::unary_operator_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        node->token = token;
        node->type = ast_node_types_t::unary_operator;
        return node;
    }

    ast_node_shared_ptr ast_builder::string_literal_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        node->token = token;
        node->type = ast_node_types_t::string_literal;
        return node;
    }

    ast_node_shared_ptr ast_builder::number_literal_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        node->token = token;
        node->type = ast_node_types_t::number_literal;
        return node;
    }

    ast_node_shared_ptr ast_builder::boolean_literal_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        node->token = token;
        node->type = ast_node_types_t::boolean_literal;
        return node;
    }

    ast_node_shared_ptr ast_builder::type_identifier_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        node->token = token;
        node->type = ast_node_types_t::type_identifier;
        return node;
    }

    ast_node_shared_ptr ast_builder::character_literal_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        node->token = token;
        node->type = ast_node_types_t::character_literal;
        return node;
    }

    ast_node_shared_ptr ast_builder::variable_reference_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        node->token = token;
        node->type = ast_node_types_t::variable_reference;
        return node;
    }

    ast_node_shared_ptr ast_builder::variable_declaration_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        node->token = token;
        node->type = ast_node_types_t::variable_declaration;
        return node;
    }

};