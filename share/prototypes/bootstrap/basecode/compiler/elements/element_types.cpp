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

#include <sstream>
#include <fmt/format.h>
#include <common/string_support.h>
#include <compiler/element_builder.h>
#include "type.h"
#include "field.h"
#include "attribute.h"
#include "identifier.h"
#include "pointer_type.h"
#include "element_types.h"
#include "symbol_element.h"
#include "type_reference.h"

namespace basecode::compiler {

    ///////////////////////////////////////////////////////////////////////////

    void attribute_map_t::add(attribute* value) {
        auto attr = find(value->name());
        if (attr != nullptr)
            return;
        _attrs.insert(std::make_pair(value->name(), value));
    }

    attribute_list_t attribute_map_t::as_list() const {
        attribute_list_t list {};
        for (const auto& it : _attrs) {
            list.push_back(it.second);
        }
        return list;
    }

    bool attribute_map_t::remove(const std::string& name) {
        return _attrs.erase(name) > 0;
    }

    compiler::attribute* attribute_map_t::find(const std::string& name) const {
        auto it = _attrs.find(name);
        if (it != _attrs.end())
            return it->second;
        return nullptr;
    }

    ///////////////////////////////////////////////////////////////////////////

    void field_map_t::add(field* value) {
        _fields.insert(std::make_pair(value->id(), value));
    }

    field_list_t field_map_t::as_list() const {
        field_list_t list {};
        for (const auto& it : _fields) {
            list.push_back(it.second);
        }
        return list;
    }

    bool field_map_t::remove(common::id_t id) {
        return _fields.erase(id) > 0;
    }

    compiler::field* field_map_t::find(common::id_t id) {
        auto it = _fields.find(id);
        if (it != _fields.end())
            return it->second;
        return nullptr;
    }

    compiler::field* field_map_t::find_by_name(const std::string& name) {
        for (const auto& kvp : _fields) {
            if (kvp.second->identifier()->symbol()->name() == name)
                return kvp.second;
        }
        return nullptr;
    }

    ///////////////////////////////////////////////////////////////////////////

    void identifier_map_t::dump() {
        for (const auto& it : _identifiers) {
            fmt::print("{0} := id({1})\n", it.first, it.second->id());
        }
    }

    void identifier_map_t::add(identifier* value) {
        // XXX:
        _identifiers.insert(std::make_pair(value->symbol()->name(), value));
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

    identifier_list_t identifier_map_t::find(const std::string& name) {
        identifier_list_t list {};
        auto range = _identifiers.equal_range(name);
        for (auto it = range.first; it != range.second; ++it)
            list.emplace_back(it->second);
        return list;
    }

    ///////////////////////////////////////////////////////////////////////////

    void type_map_t::add(
            compiler::symbol_element* symbol,
            compiler::type* type) {
        _types.insert(std::make_pair(symbol->name(), type));
    }

    type_list_t type_map_t::as_list() const {
        type_list_t list {};
        for (const auto& it : _types) {
            list.push_back(it.second);
        }
        return list;
    }

    void type_map_t::add(compiler::type* type) {
        _types.insert(std::make_pair(type->symbol()->name(), type));
    }

    string_list_t type_map_t::name_list() const {
        string_list_t names {};
        for (const auto& it : _types) {
            names.push_back(it.first);
        }
        return names;
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

    ///////////////////////////////////////////////////////////////////////////

    std::string make_fully_qualified_name(const symbol_element* symbol) {
        std::stringstream stream {};
        auto count = 0;
        for (const auto& name : symbol->namespaces()) {
            if (count > 0)
                stream << "::";
            stream << name;
            count++;
        }
        if (count > 0)
            stream << "::";
        stream << symbol->name();
        return stream.str();
    }

    std::string make_fully_qualified_name(const qualified_symbol_t& symbol) {
        std::stringstream stream {};
        auto count = 0;
        for (const auto& name : symbol.namespaces) {
            if (count > 0)
                stream << "::";
            stream << name;
            count++;
        }
        if (count > 0)
            stream << "::";
        stream << symbol.name;
        return stream.str();
    }

    ///////////////////////////////////////////////////////////////////////////

    std::string infer_type_result_t::type_name() const {
        if (reference != nullptr)
            return reference->symbol().name;
        return inferred_type->symbol()->name();
    }

    compiler::type* infer_type_result_t::base_type() const {
        if (inferred_type == nullptr)
            return nullptr;
        if (inferred_type->is_pointer_type()) {
            auto pointer_type = dynamic_cast<compiler::pointer_type*>(inferred_type);
            return pointer_type->base_type_ref()->type();
        } else {
            return inferred_type;
        }
    }

    qualified_symbol_t make_qualified_symbol(const std::string& symbol) {
        qualified_symbol_t qs {};

        size_t index = 0;
        std::stringstream stream;
        while (index < symbol.length()) {
            const auto& c = symbol[index];
            if (c == ':') {
                ++index;
                if (c == ':') {
                    qs.namespaces.emplace_back(stream.str());
                    stream.str("");
                }
            } else {
                stream << c;
            }
            ++index;
        }
        qs.name = stream.str();
        qs.fully_qualified_name = symbol;

        return qs;
    }

    compiler::type_reference* type_find_result_t::make_type_reference(
            element_builder& builder,
            compiler::block* parent_scope) {
        return builder.make_type_reference(parent_scope, type_name, type);
    }
};