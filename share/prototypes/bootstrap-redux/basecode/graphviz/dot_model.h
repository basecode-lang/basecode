// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
//       C O M P I L E R  P R O J E C T
//
// Copyright (C) 2019 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <string_view>
#include <basecode/strings/pool.h>
#include <basecode/adt/hash_table.h>
#include "model.h"

namespace basecode::graphviz {

    using namespace std::literals;

    class dot_model_t final : public model_t {
        struct metadata_t final {
            std::string_view name;
            uint8_t component_flags{};
        };

        using metadata_table_t = adt::hash_table_t<attribute_type_t, metadata_t>;

    public:
        explicit dot_model_t(memory::allocator_t* allocator = context::current()->allocator);

        bool serialize(
            result_t& r,
            graph_t& graph,
            format::memory_buffer_t& buffer) override;

        bool is_attribute_valid(
            result_t& r,
            component_type_t component,
            attribute_type_t type) override;

        bool initialize(result_t& r) override;

        std::string_view attribute_type_to_name(attribute_type_t type) override;

    private:
        bool serialize_attribute(
            result_t& r,
            attribute_value_t* attr,
            format::memory_buffer_t& buffer);

        bool serialize_attributes(
            result_t& r,
            const attribute_value_list_t& attrs,
            std::string_view separator,
            format::memory_buffer_t& buffer);

        adt::string_t escape_chars(const adt::string_t& value);

    private:
        metadata_table_t _attributes;
        memory::allocator_t* _allocator;
    };

}