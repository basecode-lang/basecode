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

#include "attribute.h"

namespace basecode::graphviz {

    std::string_view component_type_to_name(component_type_t type) {
        switch (type) {
            case component_type_t::edge:                return "edge"sv;
            case component_type_t::node:                return "node"sv;
            case component_type_t::graph:               return "graph"sv;
            case component_type_t::subgraph:            return "subgraph"sv;
            case component_type_t::cluster_subgraph:    return "cluster_subgraph"sv;
            default:                                    return "unknown"sv;
        }
    }

}