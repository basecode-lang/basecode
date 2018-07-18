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

#include <functional>
#include <parser/ast.h>
#include <common/id_pool.h>
#include <vm/assembly_listing.h>
#include "element.h"
#include "element_map.h"

namespace basecode::compiler {

    using block_visitor_callable = std::function<bool (compiler::block*)>;

    struct type_find_result_t {
        qualified_symbol_t type_name;
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
            vm::assembly_listing& listing,
            const syntax::ast_node_shared_ptr& root);

        bool compile_module(
            common::result& r,
            vm::assembly_listing& listing,
            const syntax::ast_node_shared_ptr& root);

        element_map& elements();

        bool run(common::result& r);

        compiler::type* find_type(const qualified_symbol_t& symbol) const;

    protected:
        friend class code_dom_formatter;

        vm::terp* terp();

        compiler::block* block();

    private:
        friend class any_type;
        friend class directive;
        friend class type_info;
        friend class array_type;
        friend class tuple_type;
        friend class string_type;
        friend class numeric_type;
        friend class unary_operator;
        friend class namespace_type;
        friend class procedure_type;
        friend class binary_operator;

        bool on_emit(
            common::result& r,
            emit_context_t& context) override;

        bool visit_blocks(
            common::result& r,
            const block_visitor_callable& callable,
            compiler::block* root_block = nullptr);

        void initialize_core_types(common::result& r);

        bool resolve_unknown_types(common::result& r);

        bool resolve_unknown_identifiers(common::result& r);

    private:
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
            compiler::block* parent_scope,
            compiler::block* scope);

        type_info* make_type_info_type(
            common::result& r,
            compiler::block* parent_scope,
            compiler::block* scope);

        compiler::directive* make_directive(
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

        compiler::symbol_element* make_symbol(
            compiler::block* parent_scope,
            const std::string& name,
            const string_list_t& namespaces = {});

        compiler::symbol_element* make_temp_symbol(
            compiler::block* parent_scope,
            const std::string& name,
            const string_list_t& namespaces);

        void add_procedure_instance(
            common::result& r,
            compiler::procedure_type* proc_type,
            const syntax::ast_node_shared_ptr& node);

        identifier* make_identifier(
            compiler::block* parent_scope,
            compiler::symbol_element* symbol,
            initializer* expr);

        string_literal* make_string(
            compiler::block* parent_scope,
            const std::string& value);

        array_type* make_array_type(
            common::result& r,
            compiler::block* parent_scope,
            compiler::block* scope,
            compiler::type* entry_type,
            size_t size);

        expression* make_expression(
            compiler::block* parent_scope,
            element* expr);

        void add_expression_to_scope(
            compiler::block* scope,
            compiler::element* expr);

        tuple_type* make_tuple_type(
            common::result& r,
            compiler::block* parent_scope,
            compiler::block* scope);

        initializer* make_initializer(
            compiler::block* parent_scope,
            element* expr);

        integer_literal* make_integer(
            compiler::block* parent_scope,
            uint64_t value);

        string_type* make_string_type(
            common::result& r,
            compiler::block* parent_scope,
            compiler::block* scope);

        void add_composite_type_fields(
            common::result& r,
            compiler::composite_type* type,
            const syntax::ast_node_shared_ptr& block);

        composite_type* make_enum_type(
            common::result& r,
            compiler::block* parent_scope,
            compiler::block* scope);

        compiler::type* find_array_type(
            compiler::type* entry_type,
            size_t size);

        unknown_type* make_unknown_type(
            common::result& r,
            compiler::block* parent_scope,
            compiler::symbol_element* symbol,
            bool is_array,
            size_t array_size);

        composite_type* make_union_type(
            common::result& r,
            compiler::block* parent_scope,
            compiler::block* scope);

        numeric_type* make_numeric_type(
            common::result& r,
            compiler::block* parent_scope,
            const std::string& name,
            int64_t min,
            uint64_t max);

        composite_type* make_struct_type(
            common::result& r,
            compiler::block* parent_scope,
            compiler::block* scope);

        namespace_element* make_namespace(
            compiler::block* parent_scope,
            element* expr);

        namespace_type* make_namespace_type(
            common::result& r,
            compiler::block* parent_scope);

        procedure_call* make_procedure_call(
            compiler::block* parent_scope,
            compiler::identifier_reference* reference,
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

        identifier_reference* make_identifier_reference(
            compiler::block* parent_scope,
            const qualified_symbol_t& symbol,
            compiler::identifier* identifier);

        procedure_instance* make_procedure_instance(
            compiler::block* parent_scope,
            compiler::type* procedure_type,
            compiler::block* scope);

        compiler::element* resolve_symbol_or_evaluate(
            common::result& r,
            const syntax::ast_node_shared_ptr& node);

        compiler::identifier* add_identifier_to_scope(
            common::result& r,
            compiler::symbol_element* symbol,
            type_find_result_t& find_type_result,
            const syntax::ast_node_shared_ptr& node,
            compiler::block* parent_scope = nullptr);

        void add_type_to_scope(compiler::type* type);

        void make_qualified_symbol(
            qualified_symbol_t& symbol,
            const syntax::ast_node_shared_ptr& node);

        compiler::symbol_element* make_symbol_from_node(
            common::result& r,
            const syntax::ast_node_shared_ptr& node);

        unknown_type* make_unknown_type_from_find_result(
            common::result& r,
            compiler::block* scope,
            compiler::identifier* identifier,
            const type_find_result_t& result);

        return_element* make_return(compiler::block* parent_scope);

        argument_list* make_argument_list(compiler::block* parent_scope);

        compiler::block* push_new_block(element_type_t type = element_type_t::block);

    private:
        element* evaluate_in_scope(
            common::result& r,
            const syntax::ast_node_shared_ptr& node,
            compiler::block* scope,
            element_type_t default_block_type = element_type_t::block);

        element* evaluate(
            common::result& r,
            const syntax::ast_node_shared_ptr& node,
            element_type_t default_block_type = element_type_t::block);

        bool find_identifier_type(
            common::result& r,
            type_find_result_t& result,
            const syntax::ast_node_shared_ptr& type_node,
            compiler::block* parent_scope = nullptr);

        compiler::block* pop_scope();

        compiler::block* current_scope() const;

        void push_scope(compiler::block* block);

        bool is_subtree_constant(const syntax::ast_node_shared_ptr& node);

        compiler::identifier* find_identifier(const qualified_symbol_t& symbol);

        bool within_procedure_scope(compiler::block* parent_scope = nullptr) const;

    private:
        vm::assembler _assembler;
        element_map _elements {};
        vm::terp* _terp = nullptr;
        compiler::block* _block = nullptr;
        std::stack<compiler::block*> _scope_stack {};
        identifier_list_t _identifiers_with_unknown_types {};
        identifier_reference_list_t _unresolved_identifier_references {};
        std::unordered_map<std::string, string_literal_list_t> _interned_string_literals {};
    };

};

