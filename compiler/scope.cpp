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

#include "scope.h"

namespace basecode::compiler {

    scope::scope(
        compiler::scope* parent,
        const syntax::ast_node_shared_ptr& node) : _parent(parent),
                                                   _node(node) {
    }

    void scope::clear() {
        _children.clear();
        _symbol_table.clear();
    }

    uint64_t scope::address() const {
        return _address;
    }

    compiler::scope* scope::parent() {
        return _parent;
    }

    void scope::address(uint64_t value) {
        _address = value;
    }

    syntax::ast_node_shared_ptr scope::ast_node() {
        return _node;
    }

    const scope_list& scope::children() const {
        return _children;
    }

    compiler::scope* scope::add_child_scope(const syntax::ast_node_shared_ptr& node) {
        auto scope = std::make_unique<compiler::scope>(this, node);
        _children.push_back(std::move(scope));
        return scope.get();
    }

};