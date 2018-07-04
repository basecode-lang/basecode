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

    any_type::any_type(
        element* parent) : compiler::type(
                                parent,
                                element_type_t::any_type,
                                "any") {
    }

    compiler::type* any_type::underlying_type() {
        return _underlying_type;
    }

    bool any_type::on_initialize(common::result& r) {
        return true;
    }

    void any_type::underlying_type(compiler::type* value) {
        _underlying_type = value;
    }

};