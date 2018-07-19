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

#include "field.h"
#include "block.h"
#include "identifier.h"
#include "composite_type.h"

namespace basecode::compiler {

    composite_type::composite_type(
            block* parent_scope,
            composite_types_t type,
            compiler::block* scope,
            compiler::symbol_element* symbol,
            element_type_t element_type) : compiler::type(
                                                parent_scope,
                                                element_type,
                                                symbol),
                                           _type(type),
                                           _scope(scope) {
    }

    bool composite_type::on_initialize(
            common::result& r,
            compiler::program* program) {
        size_t size = 0;
        for (auto fld : _fields.as_list())
            size += fld->identifier()->type()->size_in_bytes();
        size_in_bytes(size);
        return true;
    }

    field_map_t& composite_type::fields() {
        return _fields;
    }

    compiler::block* composite_type::scope() {
        return _scope;
    }

    bool composite_type::on_is_constant() const {
        return true;
    }

    type_map_t& composite_type::type_parameters() {
        return _type_parameters;
    }

    composite_types_t composite_type::type() const {
        return _type;
    }

    void composite_type::on_owned_elements(element_list_t& list) {
        for (auto element : _fields.as_list())
            list.emplace_back(element);

        if (_scope != nullptr)
            list.emplace_back(_scope);
    }

};