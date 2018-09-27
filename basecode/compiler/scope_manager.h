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

#include <cstdint>
#include <functional>
#include <parser/ast.h>
#include "compiler_types.h"

namespace basecode::compiler {

    using block_stack_t = std::stack<compiler::block*>;
    using module_stack_t = std::stack<compiler::module*>;

    using block_visitor_callable = std::function<bool (compiler::block*)>;
    using scope_visitor_callable = std::function<compiler::element* (compiler::block*)>;
    using element_visitor_callable = std::function<compiler::element* (compiler::element*)>;
    using namespace_visitor_callable = std::function<compiler::element* (compiler::block*)>;

    class scope_manager {
    public:
        explicit scope_manager(compiler::session& session);

        bool visit_blocks(
            common::result& r,
            const block_visitor_callable& callable,
            compiler::block* root_block = nullptr);

        compiler::type* find_type(
            const qualified_symbol_t& symbol,
            compiler::block* scope = nullptr) const;

        bool find_identifier_type(
            type_find_result_t& result,
            const syntax::ast_node_t* type_node,
            const element_list_t& array_subscripts,
            compiler::block* parent_scope = nullptr);

        compiler::block* pop_scope();

        element* walk_parent_scopes(
            compiler::block* scope,
            const scope_visitor_callable& callable) const;

        module_stack_t& module_stack();

        element* walk_parent_elements(
            compiler::element* element,
            const element_visitor_callable& callable) const;

        element* walk_qualified_symbol(
            const qualified_symbol_t& symbol,
            compiler::block* scope,
            const namespace_visitor_callable& callable) const;

        compiler::type* find_array_type(
            compiler::type* entry_type,
            const element_list_t& subscripts,
            compiler::block* scope = nullptr) const;

        compiler::type* find_pointer_type(
            compiler::type* base_type,
            compiler::block* scope = nullptr) const;

        block_stack_t& top_level_stack();

        compiler::module* current_module();

        compiler::block* current_top_level();

        compiler::block* current_scope() const;

        void push_scope(compiler::block* block);

        compiler::identifier* find_identifier(
            const qualified_symbol_t& symbol,
            compiler::block* scope = nullptr) const;

        void add_type_to_scope(compiler::type* type);

        identifier_list_t& identifiers_with_unknown_types();

        identifier_reference_list_t& unresolved_identifier_references();

        compiler::module* find_module(compiler::element* element) const;

        bool within_procedure_scope(compiler::block* parent_scope = nullptr) const;

        compiler::block* push_new_block(element_type_t type = element_type_t::block);

    private:
        compiler::session& _session;
        block_stack_t _scope_stack {};
        module_stack_t _module_stack {};
        block_stack_t _top_level_stack {};
        identifier_list_t _identifiers_with_unknown_types {};
        identifier_reference_list_t _unresolved_identifier_references {};
    };

};
