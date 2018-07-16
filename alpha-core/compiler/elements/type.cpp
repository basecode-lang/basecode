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

namespace basecode::compiler {

    type::type(
        block* parent_scope,
        element_type_t type,
        compiler::symbol_element* symbol) : element(parent_scope, type),
                                    _symbol(symbol) {
    }

    bool type::initialize(
            common::result& r,
            compiler::program* program) {
        return on_initialize(r, program);
    }

    bool type::on_initialize(
            common::result& r,
            compiler::program* program) {
        return true;
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

    compiler::symbol_element* type::symbol() const {
        return _symbol;
    }

    void type::symbol(compiler::symbol_element* value) {
        _symbol = value;
    }

};