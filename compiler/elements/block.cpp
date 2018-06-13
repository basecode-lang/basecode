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

#include "block.h"

namespace basecode::compiler {

    block::block(block* parent) : _parent(parent) {
    }

    block::~block() {
    }

    block* block::parent() const {
        return _parent;
    }

    element_list_t& block::children() {
        return _children;
    }

    bool block::remove_type(const std::string& name) {
        return _types.erase(name) > 0;
    }

    bool block::remove_identifier(const std::string& name) {
        return _identifiers.erase(name) > 0;
    }

    compiler::type* block::find_type(const std::string& name) {
        auto it = _types.find(name);
        if (it != _types.end())
            return it->second;
        return nullptr;
    }

    compiler::identifier* block::find_identifier(const std::string& name) {
        auto it = _identifiers.find(name);
        if (it != _identifiers.end())
            return it->second;
        return nullptr;
    }

};