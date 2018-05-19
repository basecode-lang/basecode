#include "scope.h"

namespace basecode {

    scope::scope(
        basecode::scope* parent,
        const ast_node_shared_ptr& node) : _parent(parent),
                                           _node(node) {
    }

    void scope::clear() {
        _children.clear();
        _symbol_table.clear();
    }

    uint64_t scope::address() const {
        return _address;
    }

    basecode::scope* scope::parent() {
        return _parent;
    }

    void scope::address(uint64_t value) {
        _address = value;
    }

    ast_node_shared_ptr scope::ast_node() {
        return _node;
    }

    const scope_list& scope::children() const {
        return _children;
    }

    basecode::scope* scope::add_child_scope(const ast_node_shared_ptr& node) {
        auto scope = std::make_unique<basecode::scope>(this, node);
        _children.push_back(std::move(scope));
        return scope.get();
    }

};