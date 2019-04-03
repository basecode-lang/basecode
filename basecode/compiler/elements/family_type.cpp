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

#include "family_type.h"
#include "type_reference.h"

namespace basecode::compiler {

    family_type::family_type(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::symbol_element* symbol,
            compiler::type_reference_list_t types) : type(module,
                                                          parent_scope,
                                                          element_type_t::family_type,
                                                          symbol),
                                                     _types(std::move(types)) {
    }

    bool family_type::on_type_check(
            compiler::type* other,
            const type_check_options_t& options) {
        if (other == nullptr)
            return false;

        if (id() == other->id())
            return true;

        for (auto ref : _types) {
            if (ref->type()->type_check(other, options))
                return true;
        }

        return false;
    }

    bool family_type::on_is_constant() const {
        return true;
    }

    bool family_type::is_family_type() const {
        return true;
    }

    bool family_type::on_initialize(compiler::session& session) {
        return type::on_initialize(session);
    }

    const compiler::type_reference_list_t& family_type::types() const {
        return _types;
    }

}