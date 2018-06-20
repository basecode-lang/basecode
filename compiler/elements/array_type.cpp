// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------


#include "array_type.h"

namespace basecode::compiler {

    array_type::array_type(
            element* parent,
            const std::string& name,
            compiler::type* entry_type) : compiler::type(parent, element_type_t::array_type, name),
                                          _entry_type(entry_type) {
    }

    uint64_t array_type::size() const {
        return _size;
    }

    void array_type::size(uint64_t value) {
        _size = value;
    }

    compiler::type* array_type::entry_type() {
        return _entry_type;
    }

};