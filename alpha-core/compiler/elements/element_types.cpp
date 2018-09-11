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
#include <compiler/session.h>
#include "type.h"
#include "field.h"
#include "attribute.h"
#include "identifier.h"
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

    compiler::attribute* attribute_map_t::find(const std::string& name) {
        auto it = _attrs.find(name);
        if (it != _attrs.end())
            return it->second;
        return nullptr;
    }

    ///////////////////////////////////////////////////////////////////////////

    void field_map_t::add(field* value) {
        _fields.insert(std::make_pair(value->id(), value));
    }

    field_list_t field_map_t::as_list() {
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

    identifier* identifier_map_t::find(const std::string& name) {
        auto it = _identifiers.find(name);
        if (it != _identifiers.end())
            return it->second;
        return nullptr;
    }

    ///////////////////////////////////////////////////////////////////////////

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

    element_register_t::~element_register_t() {
        if (session == nullptr)
            return;
        if (clean_up) {
            if (var != nullptr) {
                var->make_dormant(*session);
            } else {
                session->assembler().free_reg(reg);
            }
        }
    }

    vm::op_sizes element_register_t::size() const {
        if (var != nullptr) {
            return vm::op_size_for_byte_size(var->type->size_in_bytes());
        }
        return vm::op_sizes::qword;
    }

    ///////////////////////////////////////////////////////////////////////////

    std::string infer_type_result_t::type_name() const {
        if (reference != nullptr)
            return reference->symbol().name;
        return inferred_type->symbol()->name();
    }

    compiler::type_reference* type_find_result_t::make_type_reference(
            element_builder& builder,
            compiler::block* parent_scope) {
        return builder.make_type_reference(parent_scope, type_name, type);
    }
};