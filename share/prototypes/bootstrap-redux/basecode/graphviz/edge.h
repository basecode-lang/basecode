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

#include "attribute_container.h"

namespace basecode::graphviz {

    class node_t;

    class edge_t final {
    public:
        edge_t(
            memory::allocator_t* allocator,
            model_t* model,
            node_t* first,
            node_t* second);

        attribute_container_t& attributes();

        [[nodiscard]] const node_t* first() const;

        [[nodiscard]] const node_t* second() const;

    private:
        node_t* _first;
        node_t* _second;
        attribute_container_t _attributes;
    };

}