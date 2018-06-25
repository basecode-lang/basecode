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

#include "type.h"
#include "field.h"
#include "attribute.h"
#include "identifier.h"
#include "element_types.h"


namespace basecode::compiler {

    ///////////////////////////////////////////////////////////////////////////

    void attribute_map_t::add(attribute* value) {
        _attrs.insert(std::make_pair(value->name(), value));
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

    ///////////////////////////////////////////////////////////////////////////

    void field_map_t::add(field* value) {
        _fields.insert(std::make_pair(value->name(), value));
    }

    bool field_map_t::remove(const std::string& name) {
        return _fields.erase(name) > 0;
    }

    compiler::field* field_map_t::find(const std::string& name) {
        auto it = _fields.find(name);
        if (it != _fields.end())
            return it->second;
        return nullptr;
    }

    ///////////////////////////////////////////////////////////////////////////

    void identifier_map_t::add(identifier* value) {
        _identifiers.insert(std::make_pair(value->name(), value));
    }

    identifier_list_t identifier_map_t::as_list() const {
        identifier_list_t list {};
        for (const auto& it : _identifiers) {
            list.push_back(it.second);
        }
        return list;
    }

    bool identifier_map_t::remove(const std::string& name) {
        return _identifiers.erase(name) > 0;
    }

    identifier* identifier_map_t::find(const std::string& name) {
        auto it = _identifiers.find(name);
        if (it != _identifiers.end())
            return it->second;
        return nullptr;
    }

    ///////////////////////////////////////////////////////////////////////////

    void type_map_t::add(compiler::type* type) {
        _types.insert(std::make_pair(type->name(), type));
    }

    bool type_map_t::remove(const std::string& name) {
        return _types.erase(name) > 0;
    }

    compiler::type* type_map_t::find(const std::string& name) {
        auto it = _types.find(name);
        if (it != _types.end())
            return it->second;
        return nullptr;
    }

};