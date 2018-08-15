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
#include <common/source_file.h>
#include <compiler/compiler_types.h>
#include "element.h"
#include "element_map.h"
#include "ast_evaluator.h"
#include "element_builder.h"

namespace basecode::compiler {

    using block_visitor_callable = std::function<bool (compiler::block*)>;
    using scope_visitor_callable = std::function<compiler::element* (compiler::block*)>;
    using element_visitor_callable = std::function<compiler::element* (compiler::element*)>;
    using namespace_visitor_callable = std::function<compiler::element* (compiler::block*)>;

    class program : public element {
    public:
        program();

        ~program() override;

        element_map& elements();

        element_builder& builder();

        compiler::type* find_type(
            const qualified_symbol_t& symbol,
            compiler::block* scope = nullptr) const;

        void disassemble(
            compiler::session& session,
            FILE* file);

        compiler::module* compile_module(
            compiler::session& session,
            common::source_file* source_file);

        bool compile(compiler::session& session);

    protected:
        friend class code_dom_formatter;

        compiler::block* block();

        compiler::block* current_top_level();

    private:
        friend class any_type;
        friend class bool_type;
        friend class directive;
        friend class type_info;
        friend class array_type;
        friend class tuple_type;
        friend class string_type;
        friend class module_type;
        friend class numeric_type;
        friend class pointer_type;
        friend class ast_evaluator;
        friend class symbol_element;
        friend class unary_operator;
        friend class namespace_type;
        friend class procedure_type;
        friend class binary_operator;
        friend class element_builder;

        bool visit_blocks(
            common::result& r,
            const block_visitor_callable& callable,
            compiler::block* root_block = nullptr);

        bool type_check(compiler::session& session);

        bool on_emit(compiler::session& session) override;

        bool resolve_unknown_types(compiler::session& session);

        void initialize_core_types(compiler::session& session);

        bool resolve_unknown_identifiers(compiler::session& session);

    private:
        compiler::type* find_array_type(
            compiler::type* entry_type,
            size_t size,
            compiler::block* scope = nullptr);

        compiler::type* find_pointer_type(
            compiler::type* base_type,
            compiler::block* scope = nullptr);

        void add_type_to_scope(compiler::type* type);

        compiler::block* push_new_block(element_type_t type = element_type_t::block);

    private:
        bool find_identifier_type(
            compiler::session& session,
            type_find_result_t& result,
            const syntax::ast_node_shared_ptr& type_node,
            compiler::block* parent_scope = nullptr);

        compiler::block* pop_scope();

        element* walk_parent_scopes(
            compiler::block* scope,
            const scope_visitor_callable& callable) const;

        element* walk_parent_elements(
            compiler::element* element,
            const element_visitor_callable& callable) const;

        element* walk_qualified_symbol(
            const qualified_symbol_t& symbol,
            compiler::block* scope,
            const namespace_visitor_callable& callable) const;

        compiler::identifier* find_identifier(
            const qualified_symbol_t& symbol,
            compiler::block* scope = nullptr) const;

        compiler::block* current_scope() const;

        void push_scope(compiler::block* block);

        compiler::module* find_module(compiler::element* element) const;

        bool within_procedure_scope(compiler::block* parent_scope = nullptr) const;

    private:
        element_map _elements {};
        element_builder _builder;
        ast_evaluator _ast_evaluator;
        compiler::block* _block = nullptr;
        std::stack<compiler::block*> _scope_stack {};
        std::stack<compiler::block*> _top_level_stack {};
        identifier_list_t _identifiers_with_unknown_types {};
        identifier_reference_list_t _unresolved_identifier_references {};
        std::unordered_map<std::string, string_literal_list_t> _interned_string_literals {};
    };

};

