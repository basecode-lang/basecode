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

#include "type.h"
#include "field.h"
#include "symbol_element.h"

namespace basecode::compiler {

    type::type(
        block* parent_scope,
        element_type_t type,
        compiler::symbol_element* symbol) : element(parent_scope, type),
                                            _symbol(symbol) {
    }

    bool type::packed() const {
        return _packed;
    }

    void type::packed(bool value) {
        _packed = value;
    }

    size_t type::alignment() const {
        return _alignment;
    }

    void type::alignment(size_t value) {
        _alignment = value;
    }

    size_t type::size_in_bytes() const {
        return _size_in_bytes;
    }

    void type::size_in_bytes(size_t value) {
        _size_in_bytes = value;
    }

    bool type::type_check(compiler::type* other) {
        return on_type_check(other);
    }

    type_access_model_t type::access_model() const {
        return on_access_model();
    }

    type_number_class_t type::number_class() const {
        return on_number_class();
    }

    compiler::symbol_element* type::symbol() const {
        return _symbol;
    }

    bool type::on_type_check(compiler::type* other) {
        return false;
    }

    bool type::initialize(compiler::session& session) {
        return on_initialize(session);
    }

    type_access_model_t type::on_access_model() const {
        return type_access_model_t::none;
    }

    type_number_class_t type::on_number_class() const {
        return type_number_class_t::none;
    }

    void type::symbol(compiler::symbol_element* value) {
        _symbol = value;
    }

    void type::on_owned_elements(element_list_t& list) {
        if (_symbol != nullptr)
            list.emplace_back(_symbol);
    }

    bool type::on_initialize(compiler::session& session) {
        return true;
    }

};