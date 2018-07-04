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

#include "numeric_type.h"

namespace basecode::compiler {

    type_list_t numeric_type::make_types(
            common::result& r,
            element* parent) {
        type_list_t list {};
        for (const auto& it : s_types_map) {
            auto type = new numeric_type(
                parent,
                it.first,
                it.second.min,
                it.second.max);
            type->initialize(r);
            list.push_back(type);
        }
        return list;
    }

    ///////////////////////////////////////////////////////////////////////////

    numeric_type::numeric_type(
            element* parent,
            const std::string& name,
            int64_t min,
            uint64_t max) : compiler::type(
                                parent,
                                element_type_t::numeric_type,
                                name),
                            _min(min),
                            _max(max) {
    }

    int64_t numeric_type::min() const {
        return _min;
    }

    uint64_t numeric_type::max() const {
        return _max;
    }

    bool numeric_type::on_initialize(common::result& r) {
        auto it = s_types_map.find(name());
        if (it == s_types_map.end())
            return false;
        size_in_bytes(it->second.size_in_bytes);
        return true;
    }

};