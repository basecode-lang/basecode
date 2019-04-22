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

#pragma once

#include <string>
#include <unordered_map>
#include <common/id_pool.h>
#include <common/memory_pool.h>
#include "elements/element_types.h"

namespace basecode::compiler {

    using element_by_id_map_t = std::unordered_map<common::id_t, element*>;
    using element_by_type_map_t = std::unordered_map<element_type_t, element_list_t>;

    class element_map {
    public:
        element_map();

        virtual ~element_map();

        void clear();

        void remove(common::id_t id);

        element* find(common::id_t id);

        element_by_id_map_t::iterator end();

        void add(compiler::element* element);

        element_by_id_map_t::iterator begin();

        element_by_id_map_t::const_iterator end() const;

        element_by_id_map_t::const_iterator cend() const;

        element_by_id_map_t::const_iterator begin() const;

        element_by_id_map_t::const_iterator cbegin() const;

        template <typename T>
        std::vector<T*> find_by_type(element_type_t type) {
            std::vector<T*> list {};
            auto it = _elements_by_type.find(type);
            if (it != _elements_by_type.end()) {
                const auto& index_list = it->second;
                for (auto e : index_list)
                    list.emplace_back(dynamic_cast<T*>(e));
            }
            return list;
        };

        template <typename T>
        std::vector<const T*> find_by_type(element_type_t type) const {
            std::vector<const T*> list {};
            auto it = _elements_by_type.find(type);
            if (it != _elements_by_type.end()) {
                const element_list_t& index_list = it->second;
                for (auto e : index_list)
                    list.emplace_back(dynamic_cast<const T*>(e));
            }
            return list;
        };

        const_attribute_list_t attribute_by_name(const std::string& name) const;

    private:
        void add_index_by_type(compiler::element* element);

        void remove_index_by_type(compiler::element* element);

    private:
        element_by_id_map_t _elements_by_id {};
        element_by_type_map_t _elements_by_type {};
    };

}

