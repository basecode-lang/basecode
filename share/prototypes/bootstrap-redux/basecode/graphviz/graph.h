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

#include <basecode/adt/array.h>
#include <basecode/memory/object_pool.h>
#include "edge.h"
#include "node.h"
#include "attribute_container.h"

namespace basecode::graphviz {

    enum class graph_type_t {
        undirected,
        directed
    };

    class graph_t;

    using node_list_t  = adt::array_t<node_t*>;
    using edge_list_t  = adt::array_t<edge_t*>;
    using graph_list_t = adt::array_t<graph_t*>;

    class graph_t final {
    public:
        graph_t(
            model_t* model,
            graph_type_t type,
            std::string_view name,
            graph_t* parent = nullptr,
            memory::allocator_t* allocator = context::current()->allocator);

        attribute_container_t& attributes();

        [[nodiscard]] graph_type_t type() const;

        node_t* make_node(std::string_view name);

        node_t* find_node(std::string_view name);

        [[nodiscard]] std::string_view name() const;

        [[nodiscard]] const edge_list_t& edges() const;

        [[nodiscard]] const node_list_t& nodes() const;

        edge_t* make_edge(node_t* first, node_t* second);

        graph_t* make_subgraph(graph_type_t type, std::string_view name);

    private:
        model_t* _model;
        graph_t* _parent{};
        graph_type_t _type;
        edge_list_t _edges;
        node_list_t _nodes;
        std::string_view _name;
        graph_list_t _subgraphs;
        memory::object_pool_t _storage;
        memory::allocator_t* _allocator;
        attribute_container_t _attributes;
    };

}