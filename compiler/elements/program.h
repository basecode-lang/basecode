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
        cast* make_cast(
            compiler::type* type,
            element* expr);

        field* make_field(
            const std::string& name,
            compiler::type* type,
            compiler::initializer* initializer);

        if_element* make_if(
            element* predicate,
            element* true_branch,
            element* false_branch);

        comment* make_comment(
            comment_type_t type,
            const std::string& value);

        block* make_new_block();

        directive* make_directive(
            const std::string& name,
            element* expr);

        statement* make_statement(
            label_list_t labels,
            element* expr);

        any_type* make_any_type();

        attribute* make_attribute(
            const std::string& name,
            element* expr);

        composite_type* make_enum();

        identifier* make_identifier(
            const std::string& name,
            initializer* expr);

        composite_type* make_union();

        composite_type* make_struct();

        return_element* make_return();

        string_type* make_string_type();

        alias* make_alias(element* expr);

        numeric_type* make_numeric_type(
            const std::string& name,
            int64_t min,
            uint64_t max);

        unary_operator* make_unary_operator(
            operator_type_t type,
            element* rhs);

        binary_operator* make_binary_operator(
            operator_type_t type,
            element* lhs,
            element* rhs);

        procedure_type* make_procedure_type();

        expression* make_expression(element* expr);

        label* make_label(const std::string& name);

        procedure_instance* make_procedure_instance(
            compiler::type* procedure_type,
            compiler::block* scope);

        initializer* make_initializer(element* expr);

    private:
        element* evaluate(
            common::result& r,
            const syntax::ast_node_shared_ptr& node);

        block* pop_scope();

        void initialize_core_types();

        block* current_scope() const;

        void push_scope(block* block);

        type* find_type(const std::string& name);

        bool is_subtree_constant(const syntax::ast_node_shared_ptr& node);

    private:
        std::stack<block*> _scope_stack {};
        std::unordered_map<id_t, element*> _elements {};
    };

};

