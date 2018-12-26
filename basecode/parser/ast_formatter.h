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

#include <string>
#include <cstdio>
#include "ast.h"

namespace basecode::syntax {

    class ast_formatter {
    public:
        ast_formatter(
            ast_node_t* root,
            FILE* file);

        void format(const std::string& title);

    private:
        void format_node(const ast_node_t* node);

        std::string get_vertex_name(const ast_node_t* node) const;

    private:
        FILE* _file = nullptr;
        ast_node_t* _root = nullptr;
    };

}