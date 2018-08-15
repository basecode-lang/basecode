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

#include <compiler/session.h>
#include "program.h"
#include "module_type.h"
#include "symbol_element.h"

namespace basecode::compiler {

    module_type::module_type(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::block* scope) : compiler::composite_type(
                                            module,
                                            parent_scope,
                                            composite_types_t::struct_type,
                                            scope,
                                            nullptr,
                                            element_type_t::module_type) {
    }

    bool module_type::on_initialize(compiler::session& session) {
        auto program = &session.program();
        symbol(program->builder().make_symbol(parent_scope(), "module"));
        return true;
    }

    bool module_type::on_type_check(compiler::type* other) {
        return other != nullptr
            && other->symbol()->name() == symbol()->name();
    }

};