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

#include "any_type.h"

namespace basecode::compiler {

    any_type::any_type(element* parent) : compiler::composite_type(
                                                parent,
                                                composite_types_t::struct_type,
                                                "any",
                                                element_type_t::any_type) {
    }

    bool any_type::on_initialize(
            common::result& r,
            compiler::program* program) {
        // any_type := struct {
        //      type:u32;
        //      data:address;
        // };
        return true;
    }

    compiler::type* any_type::underlying_type() {
        return _underlying_type;
    }

    void any_type::underlying_type(compiler::type* value) {
        _underlying_type = value;
    }

};