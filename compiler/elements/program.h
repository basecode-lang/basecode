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

#include <parser/ast.h>
#include "block.h"
#include "any_type.h"
#include "string_type.h"
#include "numeric_type.h"

namespace basecode::compiler {

    class program : public block {
    public:
        program();

        ~program() override;

        bool initialize(
            common::result& r,
            const syntax::ast_node_shared_ptr& root);

        element* find_element(id_t id);

    private:
        block* make_new_block();

        any_type* make_any_type();

        string_type* make_string_type();

        numeric_type* make_numeric_type(
            const std::string& name,
            int64_t min,
            uint64_t max);

    private:
        element* evaluate(
            common::result& r,
            const syntax::ast_node_shared_ptr& node);

        block* pop_scope();

        void initialize_core_types();

        block* current_scope() const;

        void push_scope(block* block);

        bool is_subtree_constant(const syntax::ast_node_shared_ptr& node);

    private:
        std::stack<block*> _scope_stack {};
        std::unordered_map<id_t, element*> _elements {};
    };

};

