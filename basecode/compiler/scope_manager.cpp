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
#include "elements.h"
#include "scope_manager.h"

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
                    if (type_identifier != nullptr && type_identifier->is_constant())
                        return type_identifier->type_ref()->type();

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
                    if (type_identifier != nullptr && type_identifier->is_constant())
                        return type_identifier->type_ref()->type();
                    return nullptr;
                }));
        }
    }

    void scope_manager::add_type_to_scope(compiler::type* type) {
        auto& builder = _session.builder();

        auto scope = current_scope();
        scope->types().add(type);

        auto type_ref = builder.make_type_reference(
            scope,
            type->symbol()->qualified_symbol(),
            type);
        auto identifier = builder.make_identifier(
            scope,
            type->symbol(),
            builder.make_initializer(scope, type_ref));
        identifier->type_ref(type_ref);
        scope->identifiers().add(identifier);
    }

    compiler::block* scope_manager::push_new_block(element_type_t type) {
        auto parent_scope = current_scope();
        auto scope_block = _session.builder().make_block(parent_scope, type);

        if (parent_scope != nullptr) {
            scope_block->parent_element(parent_scope);
            parent_scope->blocks().push_back(scope_block);
        }

        push_scope(scope_block);
        return scope_block;
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

    bool scope_manager::within_local_scope(compiler::block* parent_scope) const {
        auto block_scope = parent_scope == nullptr ? current_scope() : parent_scope;
        while (block_scope != nullptr) {
            if (block_scope->is_parent_element(element_type_t::proc_type)
            ||  block_scope->is_parent_element(element_type_t::for_e)
            ||  block_scope->is_parent_element(element_type_t::if_e)
            ||  block_scope->is_parent_element(element_type_t::while_e)) {
                return true;
            }
            block_scope = block_scope->parent_scope();
        }
        return false;
    }

    compiler::identifier* scope_manager::find_identifier(
            const qualified_symbol_t& symbol,
            compiler::block* scope) const {
        std::stack<std::string> parts {};
        parts.push(symbol.name);
        for (auto it = symbol.namespaces.rbegin();
                it != symbol.namespaces.rend();
                ++it) {
            parts.push(*it);
        }

        auto block_scope = scope != nullptr ? scope : current_scope();
        compiler::identifier* result = nullptr;
        while (!parts.empty()) {
            auto& part = parts.top();

            import_set_t imports {};
            result = dynamic_cast<compiler::identifier*>(walk_parent_scopes(
                block_scope,
                [&](compiler::block* scope) -> compiler::element* {
                    auto var = scope->identifiers().find(part);
                    if (var != nullptr)
                        return var;
                    for (auto import : scope->imports())
                        imports.insert(import);
                    return nullptr;
                }));

            if (result == nullptr) {
                for (auto import : imports) {
                    auto ref = dynamic_cast<compiler::identifier_reference*>(import->expression());

                    qualified_symbol_t import_symbol{};
                    import_symbol.name = part;

                    // XXX: i thought this could be simplified to one case
                    //      but now i'm not sure.  review.
                    auto& namespaces = ref->symbol().namespaces;
                    if (import->imported_module() != nullptr) {
                        for (size_t i = 1; i < namespaces.size(); i++)
                            import_symbol.namespaces.push_back(namespaces[i]);
                    } else {
                        import_symbol.namespaces = ref->symbol().namespaces;
                    }
                    import_symbol.namespaces.push_back(ref->symbol().name);

                    compiler::block* import_scope = nullptr;
                    if (import->imported_module() != nullptr) {
                        import_scope = import->imported_module()->reference()->scope();
                    } else {
                        import_scope = import->module()->scope();
                    }
                    result = dynamic_cast<compiler::identifier*>(walk_qualified_symbol(
                        import_symbol,
                        import_scope,
                        [&part](compiler::block* scope) {
                            return scope->identifiers().find(part);
                        }));
                    if (result != nullptr)
                        break;
                }

                if(result == nullptr)
                    break;
            }

            auto init = result->initializer();
            if (init == nullptr)
                break;

            auto expr = init->expression();
            switch (expr->element_type()) {
                case element_type_t::namespace_e: {
                    auto ns = dynamic_cast<namespace_element*>(expr);
                    block_scope = dynamic_cast<compiler::block*>(ns->expression());
                    break;
                }
                case element_type_t::module_reference: {
                    auto mod_ref = dynamic_cast<compiler::module_reference*>(expr);
                    block_scope = mod_ref->reference()->scope();
                    break;
                }
                default: {
                    return result;
                }
            }

            parts.pop();
        }

        return result;
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
                auto mod_ref = dynamic_cast<compiler::module_reference*>(expr);
                block_scope = mod_ref->reference()->scope();
            } else {
                return nullptr;
            }
        }
        return callable(block_scope);
    }

    compiler::pointer_type* scope_manager::find_pointer_type(
            compiler::type* base_type,
            compiler::block* scope) const {
        return dynamic_cast<compiler::pointer_type*>(find_type(
            qualified_symbol_t(compiler::pointer_type::name_for_pointer(base_type)),
            scope));
    }

    compiler::generic_type* scope_manager::find_generic_type(
            const type_reference_list_t& constraints,
            compiler::block* scope) const {
        return dynamic_cast<compiler::generic_type*>(find_type(
            qualified_symbol_t(compiler::generic_type::name_for_generic_type(constraints)),
            scope));
    }

    compiler::map_type* scope_manager::find_map_type(
            compiler::type_reference* key_type,
            compiler::type_reference* value_type,
            compiler::block* scope) const {
        return dynamic_cast<compiler::map_type*>(find_type(
            qualified_symbol_t(compiler::map_type::name_for_map(key_type, value_type)),
            scope));
    }

    compiler::array_type* scope_manager::find_array_type(
            compiler::type* entry_type,
            const element_list_t& subscripts,
            compiler::block* scope) const {
        return dynamic_cast<compiler::array_type*>(find_type(
            qualified_symbol_t(compiler::array_type::name_for_array(entry_type, subscripts)),
            scope));
    }

    identifier_list_t& scope_manager::identifiers_with_unknown_types() {
        return _identifiers_with_unknown_types;
    }

    compiler::namespace_type* scope_manager::find_namespace_type() const {
        return dynamic_cast<compiler::namespace_type*>(find_type(
            qualified_symbol_t("namespace")));
    }

    identifier_reference_list_t& scope_manager::unresolved_identifier_references() {
        return _unresolved_identifier_references;
    }

};