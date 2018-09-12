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
#include "elements/element_types.h"

namespace basecode::compiler {

    class element_builder {
    public:
        explicit element_builder(compiler::session& session);

        cast* make_cast(
            compiler::block* parent_scope,
            compiler::type_reference* type,
            element* expr);

        transmute* make_transmute(
            compiler::block* parent_scope,
            compiler::type_reference* type,
            element* expr);

        field* make_field(
            compiler::type* type,
            compiler::block* parent_scope,
            compiler::declaration* declaration,
            uint64_t offset,
            uint8_t padding = 0);

        label* make_label(
            compiler::block* parent_scope,
            const std::string& name);

        if_element* make_if(
            compiler::block* parent_scope,
            element* predicate,
            element* true_branch,
            element* false_branch);

        import* make_import(
            compiler::block* parent_scope,
            compiler::element* expr,
            compiler::element* from_expr,
            compiler::module* imported_module);

        declaration* make_declaration(
            compiler::block* parent_scope,
            compiler::identifier* identifier,
            compiler::binary_operator* assignment);

        module* make_module(
            compiler::block* parent_scope,
            compiler::block* scope);

        raw_block* make_raw_block(
            compiler::block* parent_scope,
            const std::string& value);

        comment* make_comment(
            compiler::block* parent_scope,
            comment_type_t type,
            const std::string& value);

        any_type* make_any_type(
            compiler::block* parent_scope,
            compiler::block* scope);

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

        identifier* make_identifier(
            compiler::block* parent_scope,
            compiler::symbol_element* symbol,
            initializer* expr);

        string_literal* make_string(
            compiler::block* parent_scope,
            const std::string& value);

        array_type* make_array_type(
            compiler::block* parent_scope,
            compiler::block* scope,
            compiler::type* entry_type,
            const qualified_symbol_t& type_name,
            size_t size);

        expression* make_expression(
            compiler::block* parent_scope,
            element* expr);

        tuple_type* make_tuple_type(
            compiler::block* parent_scope,
            compiler::block* scope);

        void make_qualified_symbol(
            qualified_symbol_t& symbol,
            const syntax::ast_node_t* node);

        module_type* make_module_type(
            compiler::block* parent_scope,
            compiler::block* scope);

        initializer* make_initializer(
            compiler::block* parent_scope,
            element* expr);

        integer_literal* make_integer(
            compiler::block* parent_scope,
            uint64_t value);

        string_type* make_string_type(
            compiler::block* parent_scope,
            compiler::block* scope);

        pointer_type* make_pointer_type(
            compiler::block* parent_scope,
            const qualified_symbol_t& type_name,
            compiler::type* base_type);

        composite_type* make_enum_type(
            compiler::block* parent_scope,
            compiler::block* scope);

        type_info* make_type_info_type(
            compiler::block* parent_scope,
            compiler::block* scope);

        unknown_type* make_unknown_type(
            compiler::block* parent_scope,
            compiler::symbol_element* symbol,
            bool is_pointer,
            bool is_array,
            size_t array_size);

        composite_type* make_union_type(
            compiler::block* parent_scope,
            compiler::block* scope);

        numeric_type* make_numeric_type(
            compiler::block* parent_scope,
            const std::string& name,
            int64_t min,
            uint64_t max,
            bool is_signed,
            type_number_class_t number_class);

        composite_type* make_struct_type(
            compiler::block* parent_scope,
            compiler::block* scope);

        namespace_element* make_namespace(
            compiler::block* parent_scope,
            element* expr);

        compiler::type* make_complete_type(
            type_find_result_t& result,
            compiler::block* parent_scope = nullptr);

        compiler::directive* make_directive(
            compiler::block* parent_scope,
            const std::string& name,
            element* expr);

        intrinsic* make_free_intrinsic(
            compiler::block* parent_scope,
            compiler::argument_list* args);

        intrinsic* make_type_of_intrinsic(
            compiler::block* parent_scope,
            compiler::argument_list* args);

        intrinsic* make_alloc_intrinsic(
            compiler::block* parent_scope,
            compiler::argument_list* args);

        intrinsic* make_align_of_intrinsic(
            compiler::block* parent_scope,
            compiler::argument_list* args);

        intrinsic* make_size_of_intrinsic(
            compiler::block* parent_scope,
            compiler::argument_list* args);

        intrinsic* make_address_of_intrinsic(
            compiler::block* parent_scope,
            compiler::argument_list* args);

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

        assembly_label* make_assembly_label(
            compiler::block* parent_scope,
            const std::string& name);

        binary_operator* make_binary_operator(
            compiler::block* parent_scope,
            operator_type_t type,
            element* lhs,
            element* rhs);

        compiler::symbol_element* make_symbol(
            compiler::block* parent_scope,
            const std::string& name,
            const string_list_t& namespaces = {});

        module_reference* make_module_reference(
            compiler::block* parent_scope,
            compiler::element* expr);

        compiler::symbol_element* make_temp_symbol(
            compiler::block* parent_scope,
            const std::string& name,
            const string_list_t& namespaces);

        procedure_instance* make_procedure_instance(
            compiler::block* parent_scope,
            compiler::type* procedure_type,
            compiler::block* scope);

        type_reference* make_type_reference(
            compiler::block* parent_scope,
            const qualified_symbol_t& symbol,
            compiler::type* type);

        identifier_reference* make_identifier_reference(
            compiler::block* parent_scope,
            const qualified_symbol_t& symbol,
            compiler::identifier* identifier);

        unknown_type* make_unknown_type_from_find_result(
            compiler::block* scope,
            const type_find_result_t& result);

        bool_type* make_bool_type(compiler::block* parent_scope);

        return_element* make_return(compiler::block* parent_scope);

        argument_list* make_argument_list(compiler::block* parent_scope);

        namespace_type* make_namespace_type(compiler::block* parent_scope);

        compiler::symbol_element* make_symbol_from_node(const syntax::ast_node_t* node);

    private:
        compiler::session& _session;
    };

};

