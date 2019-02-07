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

    class scope_manager {
    public:
        explicit scope_manager(compiler::session& session);

        bool visit_child_blocks(
            common::result& r,
            const block_visitor_callable& callable,
            compiler::block* root_block = nullptr);

        compiler::type* find_type(
            const qualified_symbol_t& symbol,
            compiler::block* scope = nullptr) const;

        compiler::block* pop_scope();

        module_stack_t& module_stack();

        compiler::block* push_new_block();

        compiler::module* current_module();

        identifier_list_t find_identifier(
            const qualified_symbol_t& symbol,
            compiler::block* scope = nullptr) const;

        visitor_result_t walk_parent_scopes(
            compiler::block* scope,
            const scope_visitor_callable& callable) const;

        compiler::array_type* find_array_type(
            compiler::type* entry_type,
            const element_list_t& subscripts,
            compiler::block* scope = nullptr) const;

        visitor_result_t walk_parent_elements(
            compiler::element* element,
            const element_visitor_callable& callable) const;

        compiler::block* current_scope() const;

        visitor_result_t walk_qualified_symbol(
            const qualified_symbol_t& symbol,
            compiler::block* scope,
            const namespace_visitor_callable& callable) const;

        void push_scope(compiler::block* block);

        compiler::pointer_type* find_pointer_type(
            compiler::type* base_type,
            compiler::block* scope = nullptr) const;

        compiler::generic_type* find_generic_type(
            const type_reference_list_t& constraints,
            compiler::block* scope = nullptr) const;

        void add_type_to_scope(compiler::type* type);

        identifier_list_t& identifiers_with_unknown_types();

        compiler::namespace_type* find_namespace_type() const;

        identifier_reference_list_t& unresolved_identifier_references();

        compiler::module* find_module(compiler::element* element) const;

        bool within_local_scope(compiler::block* parent_scope = nullptr) const;

    private:
        compiler::session& _session;
        block_stack_t _scope_stack {};
        module_stack_t _module_stack {};
        identifier_list_t _identifiers_with_unknown_types {};
        identifier_reference_list_t _unresolved_identifier_references {};
    };

};
