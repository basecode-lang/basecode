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

#include "graph.h"

namespace basecode::graphviz {

    graph_t::graph_t(
            model_t* model,
            graph_type_t type,
            std::string_view name,
            graph_t* parent,
            memory::allocator_t* allocator) : _model(model),
                                              _parent(parent),
                                              _type(type),
                                              _edges(allocator),
                                              _nodes(allocator),
                                              _name(name),
                                              _subgraphs(allocator),
                                              _storage(allocator),
                                              _allocator(allocator),
                                              _attributes(
                                                  allocator,
                                                  model,
                                                  !parent ? component_type_t::graph : component_type_t::subgraph) {
        assert(_allocator);
    }

    graph_type_t graph_t::type() const {
        return _type;
    }

    std::string_view graph_t::name() const {
        return _name;
    }

    const edge_list_t& graph_t::edges() const {
        return _edges;
    }

    const node_list_t& graph_t::nodes() const {
        return _nodes;
    }

    attribute_container_t& graph_t::attributes() {
        return _attributes;
    }

    node_t* graph_t::make_node(std::string_view name) {
        auto node = _storage.construct<node_t>(_allocator, _model, name);
        _nodes.add(node);
        return node;
    }

    node_t* graph_t::find_node(std::string_view name) {
        for (auto node : _nodes) {
            if (node->name() == name)
                return node;
        }
        return nullptr;
    }

    edge_t* graph_t::make_edge(node_t* first, node_t* second) {
        auto edge = _storage.construct<edge_t>(_allocator, _model, first, second);
        _edges.add(edge);
        return edge;
    }

    graph_t* graph_t::make_subgraph(graph_type_t type, std::string_view name) {
        auto graph = _storage.construct<graph_t>(_model, type, name, this, _allocator);
        _subgraphs.add(graph);
        return graph;
    }

}