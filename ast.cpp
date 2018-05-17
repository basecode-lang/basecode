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

    ast_node_shared_ptr ast_builder::current_scope() const {
        if (_scope_stack.empty())
            return nullptr;
        auto top = _scope_stack.top();
        return top;
    }

    ast_node_shared_ptr ast_builder::push_scope(const ast_node_shared_ptr& node) {
        _scope_stack.push(node);
        return _scope_stack.top();
    }

};