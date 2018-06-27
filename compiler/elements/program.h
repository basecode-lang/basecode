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
#include <common/id_pool.h>
#include "block.h"

namespace basecode::compiler {

    class program : public element {
    public:
        program();

        ~program() override;

        bool initialize(
            common::result& r,
            const syntax::ast_node_shared_ptr& root);

        compiler::block* block();

        const element_map_t& elements() const;

        element* find_element(common::id_t id);

    private:
        cast* make_cast(
            compiler::type* type,
            element* expr);

        if_element* make_if(
            element* predicate,
            element* true_branch,
            element* false_branch);

        comment* make_comment(
            comment_type_t type,
            const std::string& value);

        void add_enum_fields(
            common::result& r,
            compiler::composite_type* enum_type,
            const syntax::ast_node_shared_ptr& block);

        void add_struct_fields(
            common::result& r,
            compiler::composite_type* struct_type,
            const syntax::ast_node_shared_ptr& block);

        void add_union_fields(
            common::result& r,
            compiler::composite_type* union_type,
            const syntax::ast_node_shared_ptr& block);

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
            initializer* expr,
            compiler::block* block_scope = nullptr);

        return_element* make_return();

        initializer* make_initializer(
            element* expr,
            compiler::block* block_scope = nullptr);

        string_type* make_string_type();

        alias* make_alias(element* expr);

        numeric_type* make_numeric_type(
            const std::string& name,
            int64_t min,
            uint64_t max);

        compiler::block* push_new_block();

        composite_type* make_union_type();

        composite_type* make_struct_type();

        namespace_element* make_namespace(
            element* expr,
            compiler::block* block_scope = nullptr);

        procedure_call* make_procedure_call(
            compiler::identifier* identifier,
            compiler::argument_list* args);

        unary_operator* make_unary_operator(
            operator_type_t type,
            element* rhs);

        argument_list* make_argument_list();

        binary_operator* make_binary_operator(
            operator_type_t type,
            element* lhs,
            element* rhs);

        namespace_type* make_namespace_type();

        boolean_literal* make_bool(bool value);

        float_literal* make_float(double value);

        expression* make_expression(element* expr);

        label* make_label(const std::string& name);

        procedure_instance* make_procedure_instance(
            compiler::type* procedure_type,
            compiler::block* scope);

        integer_literal* make_integer(uint64_t value);

        compiler::identifier* add_identifier_to_scope(
            common::result& r,
            const syntax::ast_node_shared_ptr& symbol,
            const syntax::ast_node_shared_ptr& rhs);

        void add_procedure_instance(
            common::result& r,
            compiler::procedure_type* proc_type,
            const syntax::ast_node_shared_ptr& node);

        field* make_field(compiler::identifier* identifier);

        string_literal* make_string(const std::string& value);

        procedure_type* make_procedure_type(compiler::block* block_scope);

        compiler::block* make_block(compiler::block* parent_scope = nullptr);

        array_type* make_array_type(compiler::type* entry_type, size_t size);

        compiler::type* find_array_type(compiler::type* entry_type, size_t size);

    private:
        element* evaluate(
            common::result& r,
            const syntax::ast_node_shared_ptr& node);

        compiler::block* pop_scope();

        void initialize_core_types();

        compiler::block* current_scope() const;

        void push_scope(compiler::block* block);

        type* find_type(const std::string& name);

        bool is_subtree_constant(const syntax::ast_node_shared_ptr& node);

        compiler::identifier* find_identifier(const syntax::ast_node_shared_ptr& node);

    private:
        element_map_t _elements {};
        compiler::block* _block = nullptr;
        std::stack<compiler::block*> _scope_stack {};
    };

};

