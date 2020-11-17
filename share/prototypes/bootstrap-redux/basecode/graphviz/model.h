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

#include <basecode/result.h>
#include <basecode/format/format.h>
#include "attribute.h"

namespace basecode::graphviz {

    class graph_t;

    class model_t {
    public:
        virtual ~model_t() = default;

        virtual bool serialize(
            result_t& r,
            graph_t& graph,
            format::memory_buffer_t& buffer) = 0;

        virtual bool is_attribute_valid(
            result_t& r,
            component_type_t component,
            attribute_type_t type) = 0;

        virtual bool initialize(result_t& r) = 0;

        virtual std::string_view attribute_type_to_name(attribute_type_t type) = 0;
    };

}