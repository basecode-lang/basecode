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
#include "compiler_types.h"

namespace basecode::compiler {

    class element_builder {
    public:
        explicit element_builder(compiler::session& session);

        case_element* make_case(
            compiler::block* parent_scope,
            compiler::block* scope,
            compiler::element* expr);

        switch_element* make_switch(
            compiler::block* parent_scope,
            compiler::block* scope,
            compiler::element* expr);

        fallthrough* make_fallthrough(
            compiler::block* parent_scope,
            compiler::label* label);

        spread_operator* make_spread_operator(
            compiler::block* parent_scope,
            compiler::element* expr);

        defer_element* make_defer(
            compiler::block* parent_scope,
            compiler::element* expression);

        break_element* make_break(
            compiler::block* parent_scope,
            compiler::element* label);

        continue_element* make_continue(
            compiler::block* parent_scope,
            compiler::element* label);

        while_element* make_while(
            compiler::block* parent_scope,
            compiler::binary_operator* predicate,
            compiler::block* body);

        with* make_with(
            compiler::block* parent_scope,
            compiler::element* expr,
            compiler::block* body);

        for_element* make_for(
            compiler::block* parent_scope,
            compiler::declaration* induction_decl,
            compiler::element* expression,
            compiler::block* body);

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
            uint8_t padding = 0,
            bool is_variadic = false);

        label* make_label(
            compiler::block* parent_scope,
            const std::string& name);

        if_element* make_if(
            compiler::block* parent_scope,
            element* predicate,
            element* true_branch,
            element* false_branch,
            bool is_else_if);

        import* make_import(
            compiler::block* parent_scope,
            compiler::element* expr,
            compiler::element* from_expr,
            compiler::module_reference* imported_module);

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

        identifier* make_identifier(
            compiler::block* parent_scope,
            compiler::symbol_element* symbol,
            initializer* expr);

        string_literal* make_string(
            compiler::block* parent_scope,
            const std::string& value);

        character_literal* make_character(
            compiler::block* parent_scope,
            common::rune_t rune);

        map_type* make_map_type(
            compiler::block* parent_scope,
            compiler::type_reference* key_type,
            compiler::type_reference* value_type);

        type_literal* make_user_literal(
            compiler::block* parent_scope,
            compiler::type_reference* user_type,
            compiler::argument_list* args);

        type_literal* make_map_literal(
            compiler::block* parent_scope,
            compiler::type* map_type,
            compiler::argument_list* args);

        array_type* make_array_type(
            compiler::block* parent_scope,
            compiler::block* scope,
            compiler::type_reference* entry_type,
            const element_list_t& subscripts);

        expression* make_expression(
            compiler::block* parent_scope,
            element* expr);

        type_literal* make_tuple_literal(
            compiler::block* parent_scope,
            compiler::type* tuple_type,
            compiler::argument_list* args);

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
            compiler::symbol_element* symbol);

        composite_type* make_union_type(
            compiler::block* parent_scope,
            compiler::block* scope);

        generic_type* make_generic_type(
            compiler::block* parent_scope,
            const type_reference_list_t& constraints);

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

        compiler::directive* make_directive(
            compiler::block* parent_scope,
            const std::string& name,
            const element_list_t& params);

        intrinsic* make_copy_intrinsic(
            compiler::block* parent_scope,
            compiler::argument_list* args,
            compiler::procedure_type* proc_type);

        intrinsic* make_range_intrinsic(
            compiler::block* parent_scope,
            compiler::argument_list* args,
            compiler::procedure_type* proc_type,
            const compiler::type_reference_list_t& type_params);

        intrinsic* make_fill_intrinsic(
            compiler::block* parent_scope,
            compiler::argument_list* args,
            compiler::procedure_type* proc_type);

        intrinsic* make_free_intrinsic(
            compiler::block* parent_scope,
            compiler::argument_list* args,
            compiler::procedure_type* proc_type);

        intrinsic* make_type_of_intrinsic(
            compiler::block* parent_scope,
            compiler::argument_list* args,
            compiler::procedure_type* proc_type);

        intrinsic* make_alloc_intrinsic(
            compiler::block* parent_scope,
            compiler::argument_list* args,
            compiler::procedure_type* proc_type);

        intrinsic* make_align_of_intrinsic(
            compiler::block* parent_scope,
            compiler::argument_list* args,
            compiler::procedure_type* proc_type);

        intrinsic* make_size_of_intrinsic(
            compiler::block* parent_scope,
            compiler::argument_list* args,
            compiler::procedure_type* proc_type);

        intrinsic* make_address_of_intrinsic(
            compiler::block* parent_scope,
            compiler::argument_list* args,
            compiler::procedure_type* proc_type);

        procedure_call* make_procedure_call(
            compiler::block* parent_scope,
            compiler::identifier_reference* reference,
            compiler::argument_list* args,
            const compiler::type_reference_list_t& type_params = {});

        unary_operator* make_unary_operator(
            compiler::block* parent_scope,
            operator_type_t type,
            element* rhs);

        procedure_type* make_procedure_type(
            compiler::block* parent_scope,
            compiler::block* block_scope);

        assembly_label* make_assembly_label(
            compiler::block* parent_scope,
            compiler::identifier_reference* ref,
            compiler::type* type,
            const std::string& name,
            compiler::module* module = nullptr);

        argument_pair* make_argument_pair(
            compiler::block* parent_scope,
            compiler::element* lhs,
            compiler::element* rhs);

        binary_operator* make_binary_operator(
            compiler::block* parent_scope,
            operator_type_t type,
            element* lhs,
            element* rhs);

        compiler::symbol_element* make_symbol(
            compiler::block* parent_scope,
            const std::string& name,
            const string_list_t& namespaces = {},
            const type_reference_list_t& type_params = {});

        module_reference* make_module_reference(
            compiler::block* parent_scope,
            compiler::element* expr);

        type_literal* make_array_literal(
            compiler::block* parent_scope,
            compiler::type_reference* type_ref,
            compiler::argument_list* args);

        type_reference* make_type_reference(
            compiler::block* parent_scope,
            const qualified_symbol_t& symbol,
            compiler::type* type);

        type_reference* make_type_reference(
            compiler::block* parent_scope,
            const std::string& name,
            compiler::type* type);

        compiler::nil_literal* nil_literal();

        label_reference* make_label_reference(
            compiler::block* parent_scope,
            const std::string& name);

        compiler::boolean_literal* true_literal();

        compiler::boolean_literal* false_literal();

        procedure_instance* make_procedure_instance(
            compiler::block* parent_scope,
            compiler::procedure_type* procedure_type,
            compiler::block* scope);

        identifier_reference* make_identifier_reference(
            compiler::block* parent_scope,
            const qualified_symbol_t& symbol,
            compiler::identifier* identifier,
            bool flag_as_unresolved = true);

        compiler::symbol_element* make_symbol_from_node(
            const syntax::ast_node_t* node,
            compiler::block* scope = nullptr);

        type_reference_list_t make_tagged_type_list_from_node(
            const syntax::ast_node_t* node,
            compiler::block* scope = nullptr);

        compiler::uninitialized_literal* uninitialized_literal();

        bool_type* make_bool_type(compiler::block* parent_scope);

        rune_type* make_rune_type(compiler::block* parent_scope);

        compiler::block* make_block(compiler::block* parent_scope);

        return_element* make_return(compiler::block* parent_scope);

        assignment* make_assignment(compiler::block* parent_scope);

        argument_list* make_argument_list(compiler::block* parent_scope);

        namespace_type* make_namespace_type(compiler::block* parent_scope);

    private:
        compiler::boolean_literal* make_bool(
            compiler::block* parent_scope,
            bool value);

        compiler::nil_literal* make_nil(compiler::block* parent_scope);

        compiler::uninitialized_literal* make_uninitialized_literal(compiler::block* parent_scope);

    private:
        compiler::session& _session;
        compiler::nil_literal* _nil_literal = nullptr;
        compiler::boolean_literal* _true_literal = nullptr;
        compiler::boolean_literal* _false_literal = nullptr;
        compiler::uninitialized_literal* _uninitialized_literal = nullptr;
    };

};

