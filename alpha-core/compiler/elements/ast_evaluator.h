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

#include "element_builder.h"

namespace basecode::compiler {

    class ast_evaluator {
    public:
        ast_evaluator(
            element_builder* builder,
            compiler::program* program);

        element* evaluate(
            common::result& r,
            compiler::session& session,
            const syntax::ast_node_shared_ptr& node,
            element_type_t default_block_type = element_type_t::block);

        element* evaluate_in_scope(
            common::result& r,
            compiler::session& session,
            const syntax::ast_node_shared_ptr& node,
            compiler::block* scope,
            element_type_t default_block_type = element_type_t::block);

    private:
        void apply_attributes(
            common::result& r,
            compiler::session& session,
            compiler::element* element,
            const syntax::ast_node_shared_ptr& node);

        void add_procedure_instance(
            common::result& r,
            compiler::session& session,
            compiler::procedure_type* proc_type,
            const syntax::ast_node_shared_ptr& node);

        void add_expression_to_scope(
            compiler::block* scope,
            compiler::element* expr);

        void add_composite_type_fields(
            common::result& r,
            compiler::session& session,
            compiler::composite_type* type,
            const syntax::ast_node_shared_ptr& block);

        compiler::block* add_namespaces_to_scope(
            common::result& r,
            compiler::session& session,
            const syntax::ast_node_shared_ptr& node,
            compiler::symbol_element* symbol,
            compiler::block* parent_scope);

        compiler::element* resolve_symbol_or_evaluate(
            common::result& r,
            compiler::session& session,
            const syntax::ast_node_shared_ptr& node);

        compiler::identifier* add_identifier_to_scope(
            common::result& r,
            compiler::session& session,
            compiler::symbol_element* symbol,
            type_find_result_t& find_type_result,
            const syntax::ast_node_shared_ptr& node,
            size_t source_index,
            compiler::block* parent_scope = nullptr);

    private:
        element_builder* _builder = nullptr;
        compiler::program* _program = nullptr;
    };

};

