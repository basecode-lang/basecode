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

#include <parser/parser.h>
#include <compiler/compiler_types.h>
#include "element_types.h"

namespace basecode::compiler {

    class element_builder {
    public:
        explicit element_builder(compiler::program* program);

        compiler::type* make_complete_type(
            compiler::session& session,
            type_find_result_t& result,
            compiler::block* parent_scope = nullptr);

        cast* make_cast(
            compiler::block* parent_scope,
            compiler::type* type,
            element* expr);

        transmute* make_transmute(
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
            compiler::element* expr,
            compiler::element* from_expr,
            compiler::module* module);

        module* make_module(
            compiler::block* parent_scope,
            compiler::block* scope);

        module_reference* make_module_reference(
            compiler::block* parent_scope,
            compiler::element* expr);

        comment* make_comment(
            compiler::block* parent_scope,
            comment_type_t type,
            const std::string& value);

        any_type* make_any_type(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::block* scope);

        bool_type* make_bool_type(
            compiler::session& session,
            compiler::block* parent_scope);

        type_info* make_type_info_type(
            compiler::session& session,
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

        identifier* make_identifier(
            compiler::block* parent_scope,
            compiler::symbol_element* symbol,
            initializer* expr);

        string_literal* make_string(
            compiler::block* parent_scope,
            const std::string& value);

        array_type* make_array_type(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::block* scope,
            compiler::type* entry_type,
            size_t size);

        expression* make_expression(
            compiler::block* parent_scope,
            element* expr);

        tuple_type* make_tuple_type(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::block* scope);

        module_type* make_module_type(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::block* scope);

        initializer* make_initializer(
            compiler::block* parent_scope,
            element* expr);

        integer_literal* make_integer(
            compiler::block* parent_scope,
            uint64_t value);

        string_type* make_string_type(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::block* scope);

        pointer_type* make_pointer_type(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::type* base_type);

        composite_type* make_enum_type(
            common::result& r,
            compiler::block* parent_scope,
            compiler::block* scope);

        unknown_type* make_unknown_type(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::symbol_element* symbol,
            bool is_pointer,
            bool is_array,
            size_t array_size);

        composite_type* make_union_type(
            common::result& r,
            compiler::block* parent_scope,
            compiler::block* scope);

        numeric_type* make_numeric_type(
            compiler::session& session,
            compiler::block* parent_scope,
            const std::string& name,
            int64_t min,
            uint64_t max,
            bool is_signed,
            type_number_class_t number_class);

        composite_type* make_struct_type(
            common::result& r,
            compiler::block* parent_scope,
            compiler::block* scope);

        namespace_element* make_namespace(
            compiler::block* parent_scope,
            element* expr);

        namespace_type* make_namespace_type(
            compiler::session& session,
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

        void make_qualified_symbol(
            qualified_symbol_t& symbol,
            const syntax::ast_node_t* node);

        compiler::symbol_element* make_symbol_from_node(
            common::result& r,
            const syntax::ast_node_t* node);

        unknown_type* make_unknown_type_from_find_result(
            compiler::session& session,
            compiler::block* scope,
            compiler::identifier* identifier,
            const type_find_result_t& result);

        return_element* make_return(compiler::block* parent_scope);

        argument_list* make_argument_list(compiler::block* parent_scope);

    private:
        compiler::program* _program = nullptr;
    };

};

