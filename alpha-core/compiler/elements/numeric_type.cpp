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

#include "program.h"
#include "numeric_type.h"

namespace basecode::compiler {

    void numeric_type::make_types(
            common::result& r,
            compiler::block* parent_scope,
            compiler::program* program) {
        for (const auto& it : s_types_map) {
            auto type = program->make_numeric_type(
                r,
                parent_scope,
                it.first,
                it.second.min,
                it.second.max);
            type->initialize(r, program);
            program->add_type_to_scope(type);
        }
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

    bool numeric_type::on_initialize(
            common::result& r,
            compiler::program* program) {
        auto it = s_types_map.find(name());
        if (it == s_types_map.end())
            return false;
        size_in_bytes(it->second.size_in_bytes);
        return true;
    }

};