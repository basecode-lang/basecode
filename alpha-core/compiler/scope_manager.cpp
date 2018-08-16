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

#include "session.h"
#include "scope_manager.h"
#include "elements/type.h"
#include "elements/cast.h"
#include "elements/label.h"
#include "elements/alias.h"
#include "elements/import.h"
#include "elements/module.h"
#include "elements/comment.h"
#include "elements/program.h"
#include "elements/any_type.h"
#include "elements/bool_type.h"
#include "elements/attribute.h"
#include "elements/directive.h"
#include "elements/statement.h"
#include "elements/type_info.h"
#include "elements/expression.h"
#include "elements/identifier.h"
#include "elements/if_element.h"
#include "elements/array_type.h"
#include "elements/tuple_type.h"
#include "elements/initializer.h"
#include "elements/module_type.h"
#include "elements/string_type.h"
#include "elements/numeric_type.h"
#include "elements/unknown_type.h"
#include "elements/pointer_type.h"
#include "elements/argument_list.h"
#include "elements/float_literal.h"
#include "elements/string_literal.h"
#include "elements/unary_operator.h"
#include "elements/composite_type.h"
#include "elements/procedure_type.h"
#include "elements/return_element.h"
#include "elements/procedure_call.h"
#include "elements/namespace_type.h"
#include "elements/symbol_element.h"
#include "elements/boolean_literal.h"
#include "elements/binary_operator.h"
#include "elements/integer_literal.h"
#include "elements/module_reference.h"
#include "elements/namespace_element.h"
#include "elements/procedure_instance.h"
#include "elements/identifier_reference.h"

namespace basecode::compiler {

    scope_manager::scope_manager(compiler::session& session) : _session(session) {
    }

    compiler::block* scope_manager::pop_scope() {
        if (_scope_stack.empty())
            return nullptr;
        auto top = _scope_stack.top();
        _scope_stack.pop();
        return top;
    }

    module_stack_t& scope_manager::module_stack() {
        return _module_stack;
    }

    block_stack_t& scope_manager::top_level_stack() {
        return _top_level_stack;
    }

    compiler::module* scope_manager::current_module() {
        if (_module_stack.empty())
            return nullptr;
        return _module_stack.top();
    }

    compiler::block* scope_manager::current_top_level() {
        if (_top_level_stack.empty())
            return nullptr;
        return _top_level_stack.top();
    }

    compiler::block* scope_manager::current_scope() const {
        if (_scope_stack.empty())
            return nullptr;
        return _scope_stack.top();
    }

    compiler::block* scope_manager::push_new_block(
            compiler::session& session,
            element_type_t type) {
        auto parent_scope = current_scope();
        auto scope_block = session.builder().make_block(parent_scope, type);

        if (parent_scope != nullptr) {
            scope_block->parent_element(parent_scope);
            parent_scope->blocks().push_back(scope_block);
        }

        push_scope(scope_block);
        return scope_block;
    }

    bool scope_manager::visit_blocks(
            common::result& r,
            const block_visitor_callable& callable,
            compiler::block* root_block) {
        std::function<bool (compiler::block*)> recursive_execute =
            [&](compiler::block* scope) -> bool {
                if (!callable(scope))
                    return false;
                for (auto block : scope->blocks()) {
                    if (!recursive_execute(block))
                        return false;
                }
                return true;
            };
        return recursive_execute(root_block != nullptr ? root_block : _top_level_stack.top());
    }

    void scope_manager::add_type_to_scope(compiler::type* type) {
        current_scope()->types().add(type);
    }

    bool scope_manager::find_identifier_type(
            compiler::session& session,
            type_find_result_t& result,
            const syntax::ast_node_shared_ptr& type_node,
            compiler::block* parent_scope) {
        if (type_node == nullptr)
            return false;

        session.builder().make_qualified_symbol(
            result.type_name,
            type_node->lhs.get());
        result.array_size = 0;
        result.is_array = type_node->is_array();
        result.is_spread = type_node->is_spread();
        result.is_pointer = type_node->is_pointer();
        session.builder().make_complete_type(session, result, parent_scope);
        return result.type != nullptr;
    }

    element* scope_manager::walk_parent_scopes(
            compiler::block* scope,
            const scope_visitor_callable& callable) const {
        while (scope != nullptr) {
            auto* result = callable(scope);
            if (result != nullptr)
                return result;
            scope = scope->parent_scope();
        }
        return nullptr;
    }

    element* scope_manager::walk_parent_elements(
        compiler::element* element,
        const element_visitor_callable& callable) const {
        auto current = element;
        while (current != nullptr) {
            auto* result = callable(current);
            if (result != nullptr)
                return result;
            current = current->parent_element();
        }
        return nullptr;
    }

    void scope_manager::push_scope(compiler::block* block) {
        _scope_stack.push(block);
    }

    compiler::type* scope_manager::find_type(
            const qualified_symbol_t& symbol,
            compiler::block* scope) const {
        if (symbol.is_qualified()) {
            return dynamic_cast<compiler::type*>(walk_qualified_symbol(
                symbol,
                const_cast<compiler::scope_manager*>(this)->current_top_level(),
                [&](compiler::block* scope) -> compiler::element* {
                    auto matching_type = scope->types().find(symbol.name);
                    if (matching_type != nullptr)
                        return matching_type;

                    auto type_identifier = find_identifier(symbol, scope);
                    if (type_identifier != nullptr)
                        return type_identifier->type();

                    return nullptr;
                }));
        } else {
            return dynamic_cast<compiler::type*>(walk_parent_scopes(
                scope != nullptr ? scope : current_scope(),
                [&](compiler::block* scope) -> compiler::element* {
                    auto type = scope->types().find(symbol.name);
                    if (type != nullptr)
                        return type;
                    auto type_identifier = find_identifier(symbol, scope);
                    if (type_identifier != nullptr)
                        return type_identifier->type();
                    return nullptr;
                }));
        }
    }

    compiler::module* scope_manager::find_module(compiler::element* element) const {
        return dynamic_cast<compiler::module*>(walk_parent_elements(
            element,
            [](compiler::element* each) -> compiler::element* {
                if (each->element_type() == element_type_t::module)
                    return each;
                return nullptr;
            }));
    }

    bool scope_manager::within_procedure_scope(compiler::block* parent_scope) const {
        auto block_scope = parent_scope == nullptr ? current_scope() : parent_scope;
        while (block_scope != nullptr) {
            if (block_scope->element_type() == element_type_t::proc_type_block
                ||  block_scope->element_type() == element_type_t::proc_instance_block)
                return true;
            block_scope = block_scope->parent_scope();
        }
        return false;
    }

    compiler::identifier* scope_manager::find_identifier(
        const qualified_symbol_t& symbol,
        compiler::block* scope) const {
        if (symbol.is_qualified()) {
            return dynamic_cast<compiler::identifier*>(walk_qualified_symbol(
                symbol,
                scope,
                [&symbol](compiler::block* scope) {
                    return scope->identifiers().find(symbol.name);
                }));
        } else {
            return dynamic_cast<compiler::identifier*>(walk_parent_scopes(
                scope != nullptr ? scope : current_scope(),
                [&](compiler::block* scope) -> compiler::element* {
                    auto var = scope->identifiers().find(symbol.name);
                    if (var != nullptr)
                        return var;
                    for (auto import : scope->imports()) {
                        auto identifier_reference = dynamic_cast<compiler::identifier_reference*>(import->expression());
                        auto qualified_symbol = identifier_reference->symbol();
                        qualified_symbol.namespaces.push_back(qualified_symbol.name);
                        qualified_symbol.name = symbol.name;
                        qualified_symbol.fully_qualified_name = make_fully_qualified_name(qualified_symbol);
                        var = find_identifier(qualified_symbol, import->module()->scope());
                        if (var != nullptr)
                            return var;
                    }
                    return nullptr;
                }));
        }
    }

    element* scope_manager::walk_qualified_symbol(
            const qualified_symbol_t& symbol,
            compiler::block* scope,
            const namespace_visitor_callable& callable) const {
        auto non_const_this = const_cast<compiler::scope_manager*>(this);
        auto block_scope = scope != nullptr ? scope : non_const_this->current_top_level();
        for (const auto& namespace_name : symbol.namespaces) {
            auto var = block_scope->identifiers().find(namespace_name);
            if (var == nullptr || var->initializer() == nullptr)
                return nullptr;
            auto expr = var->initializer()->expression();
            if (expr->element_type() == element_type_t::namespace_e) {
                auto ns = dynamic_cast<namespace_element*>(expr);
                block_scope = dynamic_cast<compiler::block*>(ns->expression());
            } else if (expr->element_type() == element_type_t::module_reference) {
                auto module_reference = dynamic_cast<compiler::module_reference*>(expr);
                block_scope = module_reference->module()->scope();
            } else {
                return nullptr;
            }
        }
        return callable(block_scope);
    }

    compiler::type* scope_manager::find_pointer_type(
            compiler::type* base_type,
            compiler::block* scope) {
        return find_type(
            qualified_symbol_t {
                .name = compiler::pointer_type::name_for_pointer(base_type)
            },
            scope);
    }

    compiler::type* scope_manager::find_array_type(
            compiler::type* entry_type,
            size_t size,
            compiler::block* scope) {
        return find_type(
            qualified_symbol_t {
                .name = compiler::array_type::name_for_array(entry_type, size)
            },
            scope);
    }

    identifier_list_t& scope_manager::identifiers_with_unknown_types() {
        return _identifiers_with_unknown_types;
    }

    interned_string_literal_list_t& scope_manager::interned_string_literals() {
        return _interned_string_literals;
    }

    identifier_reference_list_t& scope_manager::unresolved_identifier_references() {
        return _unresolved_identifier_references;
    }

};