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

#include "unknown_type.h"

namespace basecode::compiler {

    unknown_type::unknown_type(
        block* parent_scope,
        compiler::symbol_element* symbol) : compiler::type(
                                        parent_scope,
                                        element_type_t::unknown_type,
                                        symbol) {
    }

    bool unknown_type::on_initialize(
            common::result& r,
            compiler::program* program) {
        return true;
    }

    bool unknown_type::is_array() const {
        return _is_array;
    }

    void unknown_type::is_array(bool value) {
        _is_array = value;
    }

    size_t unknown_type::array_size() const {
        return _array_size;
    }

    void unknown_type::array_size(size_t value) {
        _array_size = value;
    }

};