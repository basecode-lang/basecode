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

#include <string>
#include <stdio.h>
#include "ast.h"

namespace basecode::syntax {

    class ast_formatter {
    public:
        ast_formatter(
            const ast_node_shared_ptr& root,
            FILE* file);

        void format_text();

        void format_graph_viz();

    private:
        void format_text_node(
            const ast_node_shared_ptr& node,
            uint32_t level);

        void format_graph_viz_node(const ast_node_shared_ptr& node);

        std::string get_vertex_name(const ast_node_shared_ptr& node) const;

    private:
        FILE* _file = nullptr;
        ast_node_shared_ptr _root;
    };

}