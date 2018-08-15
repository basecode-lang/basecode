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

#include "program.h"
#include "identifier.h"
#include "symbol_element.h"

namespace basecode::compiler {

    symbol_element::symbol_element(
            compiler::module* module,
            compiler::block* parent_scope,
            const std::string& name,
            const string_list_t& namespaces) : element(module, parent_scope, element_type_t::symbol),
                                               _name(name),
                                               _namespaces(namespaces) {
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

    qualified_symbol_t symbol_element::qualified_symbol() const {
        qualified_symbol_t symbol {};
        symbol.name = _name;
        symbol.location = location();
        symbol.namespaces = _namespaces;
        symbol.fully_qualified_name = _fully_qualified_name;
        return symbol;
    }

    bool symbol_element::operator==(const symbol_element& other) const {
        return _fully_qualified_name == other._fully_qualified_name;
    }

    bool symbol_element::operator==(const qualified_symbol_t& other) const {
        return _fully_qualified_name == other.fully_qualified_name;
    }

    compiler::type* symbol_element::on_infer_type(const compiler::program* program) {
        auto identifier = program->find_identifier(qualified_symbol());
        if (identifier != nullptr)
            return identifier->type();
        return nullptr;
    }

};