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
#include "generic_type.h"
#include "type_reference.h"

namespace basecode::compiler {

    std::string generic_type::name_for_generic_type(
            const type_reference_list_t& constraints) {
        std::stringstream stream;
        stream << "__generic";
        for (auto c : constraints) {
            stream << "_" << c->name();
        }
        stream << "__";
        return stream.str();
    }

    generic_type::generic_type(
            compiler::module* module,
            compiler::block* parent_scope,
            const compiler::type_reference_list_t& constraints) : compiler::type(
                                                                       module,
                                                                       parent_scope,
                                                                       element_type_t::generic_type,
                                                                       nullptr),
                                                                  _constraints(constraints) {
    }

    bool generic_type::on_type_check(compiler::type* other) {
        return other != nullptr
               && other->element_type() == element_type_t::generic_type;
    }

    bool generic_type::on_initialize(compiler::session& session) {
        symbol(session.builder().make_symbol(
            parent_scope(),
            name_for_generic_type(_constraints)));
        return true;
    }

    const compiler::type_reference_list_t& generic_type::constraints() const {
        return _constraints;
    }

};