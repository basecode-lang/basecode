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

#include <compiler/elements/element.h>
#include <compiler/elements/attribute.h>
#include "element_map.h"

namespace basecode::compiler {

    element_map::element_map() {
    }

    element_map::~element_map() {
        clear();
    }

    void element_map::clear() {
        _elements_by_type.clear();
        for (const auto& it : _elements_by_id)
            delete it.second;
        _elements_by_id.clear();
    }

    void element_map::remove(common::id_t id) {
        auto element = find(id);
        if (element == nullptr)
            return;

        if (element->is_singleton()) return;

        if (!element->non_owning()) {
            element_list_t owned_elements{};
            element->owned_elements(owned_elements);
            for (auto owned : owned_elements)
                remove(owned->id());
        }

        remove_index_by_type(element);
        _elements_by_id.erase(id);

        delete element;
    }

    element* element_map::find(common::id_t id) {
        auto it = _elements_by_id.find(id);
        if (it == _elements_by_id.end())
            return nullptr;
        return it->second;
    }

    element_by_id_map_t::iterator element_map::end() {
        return _elements_by_id.end();
    }

    void element_map::add(compiler::element* element) {
        _elements_by_id.insert(std::make_pair(element->id(), element));
        add_index_by_type(element);
    }

    element_by_id_map_t::iterator element_map::begin() {
        return _elements_by_id.begin();
    }

    element_by_id_map_t::const_iterator element_map::end() const {
        return _elements_by_id.end();
    }

    element_by_id_map_t::const_iterator element_map::cend() const {
        return _elements_by_id.cend();
    }

    element_by_id_map_t::const_iterator element_map::begin() const {
        return _elements_by_id.begin();
    }

    void element_map::add_index_by_type(compiler::element* element) {
        auto it = _elements_by_type.find(element->element_type());
        if (it == _elements_by_type.end()) {
            element_list_t list {};
            list.emplace_back(element);
            _elements_by_type.insert(std::make_pair(element->element_type(), list));
        } else {
            element_list_t& list = it->second;
            list.emplace_back(element);
        }
    }

    element_by_id_map_t::const_iterator element_map::cbegin() const {
        return _elements_by_id.cbegin();
    }

    void element_map::remove_index_by_type(compiler::element* element) {
        auto it = _elements_by_type.find(element->element_type());
        if (it == _elements_by_type.end())
            return;
        element_list_t& list = it->second;
        auto element_it = std::find_if(
            list.begin(),
            list.end(),
            [&element](auto item) { return item == element; });
        if (element_it == list.end())
            return;
        list.erase(element_it);
    }

    const_attribute_list_t element_map::attribute_by_name(const std::string& name) const {
        const_attribute_list_t list {};
        auto attributes = find_by_type<compiler::attribute>(element_type_t::attribute);
        for (auto attribute : attributes)
            if (attribute->name() == name)
                list.emplace_back(attribute);
        return list;
    }

};