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
#include "rune_type.h"
#include "identifier.h"
#include "initializer.h"

namespace basecode::compiler {

    rune_type::rune_type(
            compiler::module* module,
            block* parent_scope) : compiler::type(
                                       module,
                                       parent_scope,
                                       element_type_t::rune_type,
                                       nullptr) {
    }

    bool rune_type::on_type_check(compiler::type* other) {
        return other != nullptr
               && other->element_type() == element_type_t::rune_type;
    }

    number_class_t rune_type::on_number_class() const {
        return number_class_t::integer;
    }

    bool rune_type::on_initialize(compiler::session& session) {
        symbol(session.builder().make_symbol(parent_scope(), "rune"));
        alignment(4);
        size_in_bytes(4);
        return true;
    }

};