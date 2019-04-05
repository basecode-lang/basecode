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

#include <common/bytes.h>
#include "type.h"

namespace basecode::compiler {

    struct numeric_type_properties_t {
        std::string name;
        int64_t min;
        uint64_t max;
        size_t size_in_bytes;
        bool is_signed = false;
        number_class_t number_class;
    };

    using numeric_type_map_t = std::unordered_map<std::string, numeric_type_properties_t*>;

    class numeric_type : public compiler::type {
    public:
        static void make_types(
            compiler::session& session,
            compiler::block* parent_scope);

        static std::vector<std::string> type_names();

        static std::string narrow_to_value(double value);

        static std::string narrow_to_value(uint64_t value);

        static numeric_type_properties_t* type_properties_for_value(uint64_t value);

        numeric_type(
            compiler::module* module,
            block* parent_scope,
            compiler::symbol_element* symbol,
            int64_t min,
            uint64_t max,
            bool is_signed,
            number_class_t number_class);

        int64_t min() const;

        uint64_t max() const;

        bool is_signed() const override;

    protected:
        static inline std::vector<numeric_type_properties_t> s_type_properties = {
            {"u0",   0,         0,           0, false, number_class_t::integer},
            {"u8",   0,         UINT8_MAX,   1, false, number_class_t::integer},
            {"u16",  0,         UINT16_MAX,  2, false, number_class_t::integer},
            {"u32",  0,         UINT32_MAX,  4, false, number_class_t::integer},
            {"u64",  0,         UINT64_MAX,  8, false, number_class_t::integer},
            {"s8",   INT8_MIN,  INT8_MAX,    1, true,  number_class_t::integer},
            {"s16",  INT16_MIN, INT16_MAX,   2, true,  number_class_t::integer},
            {"s32",  INT32_MIN, INT32_MAX,   4, true,  number_class_t::integer},
            {"s64",  INT64_MIN, INT64_MAX,   8, true,  number_class_t::integer},
            {"f32",  0,         UINT32_MAX,  4, true,  number_class_t::floating_point},
            {"f64",  0,         UINT64_MAX,  8, true,  number_class_t::floating_point},
        };

        static inline numeric_type_map_t s_types_map = {
            {"u0",   {&s_type_properties[0]}},
            {"u8",   {&s_type_properties[1]}},
            {"u16",  {&s_type_properties[2]}},
            {"u32",  {&s_type_properties[3]}},
            {"u64",  {&s_type_properties[4]}},
            {"s8",   {&s_type_properties[5]}},
            {"s16",  {&s_type_properties[6]}},
            {"s32",  {&s_type_properties[7]}},
            {"s64",  {&s_type_properties[8]}},
            {"f32",  {&s_type_properties[9]}},
            {"f64",  {&s_type_properties[10]}},
        };

        bool on_type_check(
            compiler::type* other,
            const type_check_options_t& options) override;

        number_class_t on_number_class() const override;

        bool on_initialize(compiler::session& session) override;

    private:
        int64_t _min;
        uint64_t _max;
        bool _is_signed;
        number_class_t _number_class;
    };

}

