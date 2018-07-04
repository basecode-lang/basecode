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

#include "string_type.h"

namespace basecode::compiler {

    string_type::string_type(
        element* parent) : compiler::type(
                                parent,
                                element_type_t::string_type,
                                "string") {
    }

    bool string_type::on_initialize(common::result& r) {
        return true;
    }

};