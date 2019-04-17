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
#include "label.h"

namespace basecode::compiler {

    label::label(
            compiler::module* module,
            block* parent_scope,
            const std::string_view& name) : element(module, parent_scope, element_type_t::label),
                                            _name(name) {
    }

    compiler::element* label::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        return session.builder().make_label(new_scope, _name);
    }

    bool label::on_is_constant() const {
        return true;
    }

    std::string_view label::name() const {
        return _name;
    }

    std::string label::label_name() const {
        return fmt::format("{}_{}", _name, id());
    }

}