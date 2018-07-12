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

#include "tuple_type.h"

namespace basecode::compiler {

    tuple_type::tuple_type(block* parent_scope) : compiler::composite_type(
                                                        parent_scope,
                                                        composite_types_t::struct_type,
                                                        "tuple",
                                                        element_type_t::tuple_type) {
    }

    bool tuple_type::on_initialize(
            common::result& r,
            compiler::program* program) {
        return true;
    }


};