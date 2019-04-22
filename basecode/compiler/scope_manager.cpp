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
#include "element_builder.h"

namespace basecode::compiler {

    scope_manager::scope_manager(compiler::session& session) : _session(session) {
    }

    void scope_manager::add_type_to_scope(
            compiler::type* type,
            compiler::block* scope) {
        auto& builder = _session.builder();

        auto block_scope = scope != nullptr ? scope : current_scope();
        block_scope->types().add(type);

        auto type_ref = builder.make_type_reference(
            block_scope,
            type->symbol()->qualified_symbol(),
            type,
            true);
        auto identifier = builder.make_identifier(
            block_scope,
            type->symbol(),
            builder.make_initializer(block_scope, type_ref));
        identifier->type_ref(type_ref);
        block_scope->identifiers().add(identifier);
    }

    bool scope_manager::visit_child_blocks(
            common::result& r,
            const block_visitor_callable& callable,
            compiler::block* root_block) {
        if (!callable)
            return false;

        root_block = root_block != nullptr ? root_block : _scope_stack.top();

        std::stack<compiler::block*> blocks {};
        blocks.push(root_block);

        while (!blocks.empty()) {
            auto current = blocks.top();
            blocks.pop();

            if (!callable(current))
                return false;

            for (auto block : current->blocks())
                blocks.push(block);
        }

        return true;
    }

    compiler::type* scope_manager::find_type(
            const qualified_symbol_t& symbol,
            compiler::block* scope) const {
        auto vars = find_identifier(symbol, scope);
        if (!vars.empty()) {
            auto identifier = vars.front();
            return identifier->type_ref()->type();
        }
        return nullptr;
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

    compiler::block* scope_manager::push_new_block() {
        auto parent_scope = current_scope();
        auto scope_block = _session.builder().make_block(current_module(), parent_scope);

        if (parent_scope != nullptr) {
            scope_block->parent_element(parent_scope);
            parent_scope->blocks().push_back(scope_block);
        }

        push_scope(scope_block);
        return scope_block;
    }

    identifier_list_t scope_manager::find_identifier(
            const qualified_symbol_t& symbol,
            compiler::block* scope) const {
        std::stack<std::string_view> parts {};
        parts.push(symbol.name);
        for (auto it = symbol.namespaces.rbegin();
                 it != symbol.namespaces.rend();
                 ++it) {
            parts.push(*it);
        }

        auto block_scope = scope != nullptr ? scope : current_scope();
        visitor_result_t result;
        while (!parts.empty()) {
            auto& part = parts.top();

            import_set_t imports {};
            result = walk_parent_scopes(
                block_scope,
                [&](compiler::block* scope) -> visitor_result_t {
                    auto vars = scope->identifiers().find(part);
                    if (!vars.empty()) {
                        return visitor_result_t(vars);
                    }
                    for (auto import : scope->imports())
                        imports.insert(import);
                    return {};
                });

            if (result.empty()) {
                for (auto import : imports) {
                    auto ref = dynamic_cast<compiler::identifier_reference*>(import->expression());

                    qualified_symbol_t import_symbol{};
                    import_symbol.name = part;

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
                    result = walk_qualified_symbol(
                        import_symbol,
                        import_scope,
                        [&part](compiler::block* scope) -> visitor_result_t {
                            auto vars = scope->identifiers().find(part);
                            return vars.empty() ? visitor_result_t{} : visitor_result_t(vars);
                        });
                    if (!result.empty())
                        break;
                }

                if(result.empty())
                    break;
            }

            auto vars = *result.data<identifier_list_t>();
            compiler::identifier* identifier = vars.empty() ? nullptr : vars.front();
            auto init = identifier == nullptr ? nullptr : identifier->initializer();
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
                    goto done;
                }
            }

            parts.pop();
        }

    done:
        switch (result.type()) {
            default: {
                return {};
            }
            case visitor_data_type_t::identifier: {
                return {*result.data<compiler::identifier*>()};
            }
            case visitor_data_type_t::identifier_list: {
                return {*result.data<compiler::identifier_list_t>()};
            }
        }
    }

    visitor_result_t scope_manager::walk_parent_scopes(
            compiler::block* scope,
            const scope_visitor_callable& callable) const {
        while (scope != nullptr) {
            auto result = callable(scope);
            if (!result.empty())
                return result;
            scope = scope->parent_scope();
        }
        return {};
    }

    compiler::module* scope_manager::current_module() {
        if (_module_stack.empty())
            return nullptr;
        return _module_stack.top();
    }

    compiler::array_type* scope_manager::find_array_type(
            compiler::type* entry_type,
            const element_list_t& subscripts,
            compiler::block* scope) const {
        auto it = _session.strings().insert(compiler::array_type::name_for_array(entry_type, subscripts));
        return dynamic_cast<compiler::array_type*>(find_type(
            qualified_symbol_t(*it.first),
            scope));
    }

    visitor_result_t scope_manager::walk_parent_elements(
            compiler::element* element,
            const element_visitor_callable& callable) const {
        auto current = element;
        while (current != nullptr) {
            auto result = callable(current);
            if (!result.empty())
                return result;
            current = current->parent_element();
        }
        return {};
    }

    visitor_result_t scope_manager::walk_qualified_symbol(
            const qualified_symbol_t& symbol,
            compiler::block* scope,
            const namespace_visitor_callable& callable) const {
        auto non_const_this = const_cast<compiler::scope_manager*>(this);
        auto block_scope = scope != nullptr ?
            scope :
            non_const_this->current_module()->scope();
        for (const auto& namespace_name : symbol.namespaces) {
            auto vars = block_scope->identifiers().find(namespace_name);
            if (vars.empty() || vars.front()->initializer() == nullptr)
                return {};
            auto expr = vars.front()->initializer()->expression();
            if (expr->element_type() == element_type_t::namespace_e) {
                auto ns = dynamic_cast<namespace_element*>(expr);
                block_scope = dynamic_cast<compiler::block*>(ns->expression());
            } else if (expr->element_type() == element_type_t::module_reference) {
                auto mod_ref = dynamic_cast<compiler::module_reference*>(expr);
                block_scope = mod_ref->reference()->scope();
            } else {
                return {};
            }
        }
        return callable(block_scope);
    }

    compiler::block* scope_manager::current_scope() const {
        if (_scope_stack.empty())
            return nullptr;
        return _scope_stack.top();
    }

    void scope_manager::push_scope(compiler::block* block) {
        _scope_stack.push(block);
    }

    compiler::pointer_type* scope_manager::find_pointer_type(
            compiler::type* base_type,
            compiler::block* scope) const {
        auto it = _session.strings().insert(compiler::pointer_type::name_for_pointer(base_type));
        return dynamic_cast<compiler::pointer_type*>(find_type(
            qualified_symbol_t(*it.first),
            scope));
    }

    compiler::generic_type* scope_manager::find_generic_type(
            const type_reference_list_t& constraints,
            compiler::block* scope) const {
        auto it = _session.strings().insert(compiler::generic_type::name_for_generic_type(constraints));
        return dynamic_cast<compiler::generic_type*>(find_type(
            qualified_symbol_t(*it.first),
            scope));
    }

    element_list_t& scope_manager::elements_with_unknown_types() {
        return _elements_with_unknown_types;
    }

    compiler::namespace_type* scope_manager::find_namespace_type() const {
        return dynamic_cast<compiler::namespace_type*>(find_type(qualified_symbol_t("namespace"sv)));
    }

    bool scope_manager::within_local_scope(compiler::block* parent_scope) const {
        auto block_scope = parent_scope == nullptr ? current_scope() : parent_scope;
        while (block_scope != nullptr) {
            if (block_scope->has_stack_frame())
                return true;
            block_scope = block_scope->parent_scope();
        }
        return false;
    }

    identifier_reference_list_t& scope_manager::unresolved_identifier_references() {
        return _unresolved_identifier_references;
    }

    compiler::module* scope_manager::find_module(compiler::element* element) const {
        auto result = walk_parent_elements(
            element,
            [](compiler::element* each) -> visitor_result_t {
                if (each->element_type() == element_type_t::module)
                    return visitor_result_t(dynamic_cast<compiler::module*>(each));
                return {};
            });
        return result.type() == visitor_data_type_t::module ?
               *result.data<compiler::module*>() :
               nullptr;
    }

}