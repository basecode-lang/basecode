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

	numeric_type_map_t numeric_type::s_types_map = {
		{ "bool",{ 0,             1,           1 } },
		{ "u8",	 { 0,             UINT8_MAX,   1 } },
		{ "u16", { 0,             UINT16_MAX,  2 } },
		{ "u32", { 0,             UINT32_MAX,  4 } },
		{ "u64", { 0,             UINT64_MAX,  8 } },
		{ "s8",  { INT8_MIN,      INT8_MAX,    1 } },
		{ "s16", { INT16_MIN,     INT16_MAX,   2 } },
		{ "s32", { INT32_MIN,     INT32_MAX,   4 } },
		{ "s64", { INT64_MIN,     INT64_MAX,   8 } },
		{ "f32", { 0,             UINT32_MAX,  4 } },
		{ "f64", { 0,             UINT64_MAX,  8 } },
	};

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
            block* parent_scope,
            compiler::symbol_element* symbol,
            int64_t min,
            uint64_t max) : compiler::type(
                                parent_scope,
                                element_type_t::numeric_type,
                                symbol),
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
        auto it = s_types_map.find(symbol()->name());
        if (it == s_types_map.end())
            return false;
        size_in_bytes(it->second.size_in_bytes);
        return true;
    }

};