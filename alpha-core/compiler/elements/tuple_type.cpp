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

#include "program.h"
#include "tuple_type.h"

namespace basecode::compiler {

    tuple_type::tuple_type(
        compiler::block* parent_scope,
        compiler::block* scope) : compiler::composite_type(
                                    parent_scope,
                                    composite_types_t::struct_type,
                                    scope,
                                    nullptr,
                                    element_type_t::tuple_type) {
    }

    bool tuple_type::on_initialize(
            common::result& r,
            compiler::program* program) {
        symbol(program->make_symbol(parent_scope(), "tuple"));
        return true;
    }

    type_access_model_t tuple_type::on_access_model() const {
        return type_access_model_t::pointer;
    }

};