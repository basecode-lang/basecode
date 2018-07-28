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

#include "type.h"
#include "program.h"

namespace basecode::compiler {

    struct numeric_type_properties_t {
        int64_t min;
        uint64_t max;
        size_t size_in_bytes;
    };

    using numeric_type_map_t = std::unordered_map<std::string, numeric_type_properties_t>;

    class numeric_type : public compiler::type {
    public:
        static void make_types(
            common::result& r,
            compiler::block* parent_scope,
            compiler::program* program);

        numeric_type(
            block* parent_scope,
            compiler::symbol_element* symbol,
            int64_t min,
            uint64_t max);

        int64_t min() const;

        uint64_t max() const;

    protected:
		static numeric_type_map_t s_types_map;

        bool on_initialize(
            common::result& r,
            compiler::program* program) override;

    private:
        int64_t _min;
        uint64_t _max;
    };

};

