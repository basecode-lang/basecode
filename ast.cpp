#include "ast.h"

namespace basecode {

    ast_builder::ast_builder() {
    }

    ast_builder::~ast_builder() {
    }

    ast_node_shared_ptr ast_builder::pop_scope() {
        if (_scope_stack.empty())
            return nullptr;
        _scope_stack.pop();
        return _scope_stack.top();
    }

    ast_node_t* ast_builder::current_scope() const {
        if (_scope_stack.empty())
            return nullptr;
        return _scope_stack.top().get();
    }

    ast_node_shared_ptr ast_builder::program_node() {
        auto node = std::make_shared<ast_node_t>();
        node->type = ast_node_types_t::program;
        push_scope(node);
        return node;
    }

    void ast_builder::push_scope(const ast_node_shared_ptr& node) {
        _scope_stack.push(node);
    }

    ast_node_shared_ptr ast_builder::line_comment_node(const token_t& token) {
        auto node = std::make_shared<ast_node_t>();
        node->token = token;
        node->type = ast_node_types_t::line_comment;
        current_scope()->children.push_back(node);
        return node;
    }

};