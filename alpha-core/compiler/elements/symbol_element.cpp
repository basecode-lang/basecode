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

#include "symbol_element.h"

namespace basecode::compiler {

    symbol_element::symbol_element(
        compiler::block* parent_scope,
        const std::string& name,
        const string_list_t& namespaces) : element(parent_scope, element_type_t::symbol),
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

    const string_list_t& symbol_element::namespaces() const {
        return _namespaces;
    }

    std::string symbol_element::fully_qualified_name() const {
        std::stringstream stream {};
        auto count = 0;
        for (const auto& name : _namespaces) {
            if (count > 0)
                stream << "::";
            stream << name;
            count++;
        }
        if (count > 0)
            stream << "::";
        stream << _name;
        return stream.str();
    }

};