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

#include "edge.h"

namespace basecode::graphviz {

    edge_t::edge_t(
            memory::allocator_t* allocator,
            model_t* model,
            node_t* first,
            node_t* second) : _first(first),
                              _second(second),
                              _attributes(allocator, model, component_type_t::edge) {
    }

    const node_t* edge_t::first() const {
        return _first;
    }

    const node_t* edge_t::second() const {
        return _second;
    }

    attribute_container_t& edge_t::attributes() {
        return _attributes;
    }

}