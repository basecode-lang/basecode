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

    struct type_find_result_t {
        std::string type_name {};
        bool is_array = false;
        size_t array_size = 0;
        compiler::type* type = nullptr;
    };

    class program : public element {
    public:
        explicit program(vm::terp* terp);

        ~program() override;

        bool compile(
            common::result& r,
            const syntax::ast_node_shared_ptr& root);

        bool compile_module(
            common::result& r,
            const syntax::ast_node_shared_ptr& root);

        bool run(common::result& r);

        const element_map_t& elements() const;

        element* find_element(common::id_t id);

        compiler::type* find_type_down(const std::string& name);

        compiler::type* find_type_up(const std::string& name) const;

    protected:
        friend class directive;
        friend class code_dom_formatter;

        vm::terp* terp();

        compiler::block* block();

    private:
        bool emit_code_blocks(
            common::result& r,
            const emit_context_t& context);

        bool execute_directives(common::result& r);

        bool build_data_segments(common::result& r);

        void initialize_core_types(common::result& r);

        bool resolve_unknown_types(common::result& r);

        bool resolve_unknown_identifiers(common::result& r);

    private:
        friend class any_type;
        friend class type_info;
        friend class array_type;
        friend class string_type;
        friend class numeric_type;

        cast* make_cast(
            compiler::block* parent_scope,
            compiler::type* type,
            element* expr);

        field* make_field(
            compiler::block* parent_scope,
            compiler::identifier* identifier);

        label* make_label(
            compiler::block* parent_scope,
            const std::string& name);

        alias* make_alias(
            compiler::block* parent_scope,
            element* expr);

        if_element* make_if(
            compiler::block* parent_scope,
            element* predicate,
            element* true_branch,
            element* false_branch);

        import* make_import(
            compiler::block* parent_scope,
            element* expr);

        comment* make_comment(
            compiler::block* parent_scope,
            comment_type_t type,
            const std::string& value);

        void apply_attributes(
            common::result& r,
            compiler::element* element,
            const syntax::ast_node_shared_ptr& node);

        any_type* make_any_type(
            common::result& r,
            compiler::block* parent_scope);

        type_info* make_type_info_type(
            common::result& r,
            compiler::block* parent_scope);

        directive* make_directive(
            compiler::block* parent_scope,
            const std::string& name,
            element* expr);

        statement* make_statement(
            compiler::block* parent_scope,
            label_list_t labels,
            element* expr);

        attribute* make_attribute(
            compiler::block* parent_scope,
            const std::string& name,
            element* expr);

        float_literal* make_float(
            compiler::block* parent_scope,
            double value);

        boolean_literal* make_bool(
            compiler::block* parent_scope,
            bool value);

        compiler::block* make_block(
            compiler::block* parent_scope,
            element_type_t type);

        void add_procedure_instance(
            common::result& r,
            compiler::procedure_type* proc_type,
            const syntax::ast_node_shared_ptr& node);

        identifier* make_identifier(
            compiler::block* parent_scope,
            const std::string& name,
            initializer* expr);

        string_literal* make_string(
            compiler::block* parent_scope,
            const std::string& value);

        array_type* make_array_type(
            common::result& r,
            compiler::block* parent_scope,
            compiler::type* entry_type,
            size_t size);

        expression* make_expression(
            compiler::block* parent_scope,
            element* expr);

        void add_expression_to_scope(
            compiler::block* scope,
            compiler::element* expr);

        initializer* make_initializer(
            compiler::block* parent_scope,
            element* expr);

        integer_literal* make_integer(
            compiler::block* parent_scope,
            uint64_t value);

        string_type* make_string_type(
            common::result& r,
            compiler::block* parent_scope);

        void add_composite_type_fields(
            common::result& r,
            compiler::composite_type* type,
            const syntax::ast_node_shared_ptr& block);

        composite_type* make_enum_type(
            common::result& r,
            compiler::block* parent_scope);

        compiler::type* find_array_type(
            compiler::type* entry_type,
            size_t size);

        unknown_type* make_unknown_type(
            common::result& r,
            compiler::block* parent_scope,
            const std::string& name,
            bool is_array,
            size_t array_size);

        composite_type* make_union_type(
            common::result& r,
            compiler::block* parent_scope);

        numeric_type* make_numeric_type(
            common::result& r,
            compiler::block* parent_scope,
            const std::string& name,
            int64_t min,
            uint64_t max);

        composite_type* make_struct_type(
            common::result& r,
            compiler::block* parent_scope);

        namespace_element* make_namespace(
            compiler::block* parent_scope,
            element* expr);

        namespace_type* make_namespace_type(
            common::result& r,
            compiler::block* parent_scope);

        procedure_call* make_procedure_call(
            compiler::block* parent_scope,
            compiler::identifier* identifier,
            compiler::argument_list* args);

        unary_operator* make_unary_operator(
            compiler::block* parent_scope,
            operator_type_t type,
            element* rhs);

        procedure_type* make_procedure_type(
            compiler::block* parent_scope,
            compiler::block* block_scope);

        binary_operator* make_binary_operator(
            compiler::block* parent_scope,
            operator_type_t type,
            element* lhs,
            element* rhs);

        procedure_instance* make_procedure_instance(
            compiler::block* parent_scope,
            compiler::type* procedure_type,
            compiler::block* scope);

        compiler::identifier* add_identifier_to_scope(
            common::result& r,
            const syntax::ast_node_shared_ptr& symbol,
            const syntax::ast_node_shared_ptr& rhs,
            compiler::block* parent_scope = nullptr);

        void add_type_to_scope(compiler::type* type);

        unknown_type* make_unknown_type_from_find_result(
            common::result& r,
            compiler::block* scope,
            compiler::identifier* identifier,
            const type_find_result_t& result);

        return_element* make_return(compiler::block* parent_scope);

        argument_list* make_argument_list(compiler::block* parent_scope);

        compiler::block* push_new_block(element_type_t type = element_type_t::block);

    private:
        element* evaluate(
            common::result& r,
            const syntax::ast_node_shared_ptr& node,
            element_type_t default_block_type = element_type_t::block);

        compiler::block* pop_scope();

        void remove_element(common::id_t id);

        compiler::block* current_scope() const;

        void push_scope(compiler::block* block);

        type_find_result_t find_identifier_type(
            common::result& r,
            const syntax::ast_node_shared_ptr& symbol);

        bool is_subtree_constant(const syntax::ast_node_shared_ptr& node);

        compiler::identifier* find_identifier(const syntax::ast_node_shared_ptr& node);

    private:
        vm::assembler _assembler;
        vm::terp* _terp = nullptr;
        element_map_t _elements {};
        compiler::block* _block = nullptr;
        std::stack<compiler::block*> _scope_stack {};
        identifier_list_t _identifiers_with_unknown_types {};
    };

};

