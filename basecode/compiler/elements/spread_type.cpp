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
#include "spread_type.h"

namespace basecode::compiler {

    spread_type::spread_type(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::type_reference* type) : compiler::type(
                                                module,
                                                parent_scope,
                                                element_type_t::spread_type,
                                                nullptr),
                                              _type_ref(type) {
    }

    bool spread_type::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.reference = _type_ref;
        result.inferred_type = _type_ref->type();
        return true;
    }

    compiler::type_reference* spread_type::type() {
        return _type_ref;
    }

    void spread_type::on_owned_elements(element_list_t& list) {
        if (_type_ref != nullptr)
            list.emplace_back(_type_ref);
    }

    bool spread_type::on_initialize(compiler::session& session) {
        symbol(session.builder().make_symbol(parent_scope(), "spread"));
        return true;
    }

};