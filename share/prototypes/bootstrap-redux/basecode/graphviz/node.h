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
#include "attribute_container.h"

namespace basecode::graphviz {

    class node_t final {
    public:
        node_t(
            memory::allocator_t* allocator,
            model_t* model,
            std::string_view name);

        attribute_container_t& attributes();

        [[nodiscard]] std::string_view name() const;

    private:
        std::string_view _name;
        attribute_container_t _attributes;
    };

}