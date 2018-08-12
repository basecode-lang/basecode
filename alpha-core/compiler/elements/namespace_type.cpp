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
#include "namespace_type.h"
#include "symbol_element.h"

namespace basecode::compiler {

    namespace_type::namespace_type(block* parent_scope) : compiler::type(
                                                                parent_scope,
                                                                element_type_t::namespace_type,
                                                                nullptr) {
    }

    bool namespace_type::on_initialize(
            common::result& r,
            compiler::program* program) {
        symbol(program->builder().make_symbol(parent_scope(), "namespace"));
        return true;
    }

};