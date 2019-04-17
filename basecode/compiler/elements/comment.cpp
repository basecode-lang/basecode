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
#include "comment.h"

namespace basecode::compiler {

    comment::comment(
            compiler::module* module,
            block* parent_scope,
            comment_type_t type,
            const std::string_view& value) : element(module, parent_scope, element_type_t::comment),
                                             _type(type),
                                             _value(value) {
    }

    compiler::element* comment::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        return session.builder().make_comment(new_scope, _type, _value);
    }

    comment_type_t comment::type() const {
        return _type;
    }

    std::string_view comment::value() const {
        return _value;
    }

}