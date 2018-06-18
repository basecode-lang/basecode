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

#include "attribute.h"

namespace basecode::compiler {

    attribute_map_t::attribute_map_t(element* parent) : _parent(parent) {
    }

    attribute_map_t::~attribute_map_t() {
        for (auto attr : _attrs)
            delete attr.second;
        _attrs.clear();
    }

    bool attribute_map_t::remove(const std::string& name) {
        return _attrs.erase(name) > 0;
    }

    compiler::attribute* attribute_map_t::find(const std::string& name) {
        auto it = _attrs.find(name);
        if (it != _attrs.end())
            return it->second;
        return nullptr;
    }

    attribute* attribute_map_t::add(const std::string& name, element* expr) {
        auto attr = new compiler::attribute(_parent, name, expr);
        _attrs.insert(std::make_pair(name, attr));
        return attr;
    }

};