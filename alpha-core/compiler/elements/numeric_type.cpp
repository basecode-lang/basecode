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
#include "symbol_element.h"

namespace basecode::compiler {

    void numeric_type::make_types(
            common::result& r,
            compiler::block* parent_scope,
            compiler::program* program) {
        for (const auto& props : s_type_properties) {
            auto type = program->make_numeric_type(
                r,
                parent_scope,
                props.name,
                props.min,
                props.max,
                props.is_signed);
            type->initialize(r, program);
            program->add_type_to_scope(type);
        }
    }

    std::string numeric_type::narrow_to_value(uint64_t value) {
        for (const auto& props : s_type_properties) {
            if (value >= props.min && value <= props.max)
                return props.name;
        }
        return "u32";
    }

    ///////////////////////////////////////////////////////////////////////////

    numeric_type::numeric_type(
            block* parent_scope,
            compiler::symbol_element* symbol,
            int64_t min,
            uint64_t max,
            bool is_signed) : compiler::type(
                                parent_scope,
                                element_type_t::numeric_type,
                                symbol),
                              _min(min),
                              _max(max),
                              _is_signed(is_signed) {
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
        auto it = s_types_map.find(symbol()->name());
        if (it == s_types_map.end())
            return false;
        alignment(it->second->size_in_bytes);
        size_in_bytes(it->second->size_in_bytes);
        return true;
    }

    bool numeric_type::is_signed() const {
        return _is_signed;
    }

};