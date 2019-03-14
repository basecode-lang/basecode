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

#include <compiler/session.h>
#include <compiler/scope_manager.h>
#include "identifier.h"
#include "symbol_element.h"
#include "type_reference.h"

namespace basecode::compiler {

    symbol_element::symbol_element(
            compiler::module* module,
            compiler::block* parent_scope,
            const std::string& name,
            const string_list_t& namespaces,
            const type_reference_list_t& type_params) : element(module, parent_scope, element_type_t::symbol),
                                                        _name(name),
                                                        _namespaces(namespaces),
                                                        _type_parameters(type_params) {
    }

    bool symbol_element::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        auto vars = session.scope_manager().find_identifier(qualified_symbol());
        compiler::identifier* identifier = vars.empty() ? nullptr : vars.front();
        if (identifier != nullptr) {
            result.inferred_type = identifier->type_ref()->type();
            result.reference = identifier->type_ref();
            return true;
        }
        return false;
    }

    std::string symbol_element::name() const {
        return _name;
    }

    void symbol_element::constant(bool value) {
        _is_constant = value;
    }

    bool symbol_element::is_qualified() const {
        return !_namespaces.empty();
    }

    bool symbol_element::on_is_constant() const {
        return _is_constant;
    }

    void symbol_element::cache_fully_qualified_name() {
        _fully_qualified_name = make_fully_qualified_name(this);
    }

    std::string symbol_element::fully_qualified_name() {
        if (_fully_qualified_name.empty())
            cache_fully_qualified_name();
        return _fully_qualified_name;
    }

    const string_list_t& symbol_element::namespaces() const {
        return _namespaces;
    }

    bool symbol_element::on_as_string(std::string& value) const {
        value = _name;
        return true;
    }

    qualified_symbol_t symbol_element::qualified_symbol() const {
        qualified_symbol_t symbol {};
        symbol.name = _name;
        symbol.location = location();
        symbol.namespaces = _namespaces;
        symbol.fully_qualified_name = _fully_qualified_name;
        return symbol;
    }

    void symbol_element::on_owned_elements(element_list_t& list) {
        for (auto type_param : _type_parameters)
            list.emplace_back(type_param);
    }

    bool symbol_element::operator==(const symbol_element& other) const {
        return _fully_qualified_name == other._fully_qualified_name;
    }

    const type_reference_list_t& symbol_element::type_parameters() const {
        return _type_parameters;
    }

    bool symbol_element::operator==(const qualified_symbol_t& other) const {
        return _fully_qualified_name == other.fully_qualified_name;
    }

}