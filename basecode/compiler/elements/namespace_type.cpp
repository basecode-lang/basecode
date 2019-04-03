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
#include <compiler/element_builder.h>
#include "namespace_type.h"
#include "symbol_element.h"

namespace basecode::compiler {

    namespace_type::namespace_type(
        compiler::module* module,
        block* parent_scope) : compiler::type(
                                    module,
                                    parent_scope,
                                    element_type_t::namespace_type,
                                    nullptr) {
    }

    bool namespace_type::on_type_check(
            compiler::type* other,
            const type_check_options_t& options) {
        return other != nullptr
               && other->id() == id();
    }

    bool namespace_type::on_initialize(compiler::session& session) {
        symbol(session.builder().make_symbol(parent_scope(), "namespace"));
        return true;
    }

}