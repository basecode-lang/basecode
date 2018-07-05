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

#pragma once

#include <vm/assembler.h>
#include "type.h"

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
            element* parent,
            const std::string& name,
            int64_t min,
            uint64_t max);

        int64_t min() const;

        uint64_t max() const;

    protected:
        static inline numeric_type_map_t s_types_map = {
            {"bool",    {0,         1,           1}},
            {"u8",      {0,         UINT8_MAX,   1}},
            {"u16",     {0,         UINT16_MAX,  2}},
            {"u32",     {0,         UINT32_MAX,  4}},
            {"u64",     {0,         UINT64_MAX,  8}},
            {"s8",      {INT8_MIN,  INT8_MAX,    1}},
            {"s16",     {INT16_MIN, INT16_MAX,   2}},
            {"s32",     {INT32_MIN, INT32_MAX,   4}},
            {"s64",     {INT64_MIN, INT64_MAX,   8}},
            {"f32",     {0,         UINT32_MAX,  4}},
            {"f64",     {0,         UINT64_MAX,  8}},
            {"address", {0,         UINTPTR_MAX, 8}},
        };

        bool on_initialize(
            common::result& r,
            compiler::program* program) override;

    private:
        int64_t _min;
        uint64_t _max;
    };

};

