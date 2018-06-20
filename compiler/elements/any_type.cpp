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
        element* parent) : compiler::type(parent,
                                          element_type_t::any_type,
                                          "any") {
    }

};