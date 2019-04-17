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
#include "identifier.h"
#include "declaration.h"
#include "binary_operator.h"

namespace basecode::compiler {

    declaration::declaration(
        compiler::module* module,
        compiler::block* parent_scope,
        compiler::identifier* identifier,
        compiler::binary_operator* assignment) : element(module, parent_scope, element_type_t::declaration),
                                                 _identifier(identifier),
                                                 _assignment(assignment) {
    }

    compiler::element* declaration::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        return session.builder().make_declaration(
            new_scope,
            _identifier->clone<compiler::identifier>(session, new_scope),
            _assignment != nullptr ?
                _assignment->clone<compiler::binary_operator>(session, new_scope) :
                nullptr);
    }

    compiler::identifier* declaration::identifier() {
        return _identifier;
    }

    compiler::binary_operator* declaration::assignment() {
        return _assignment;
    }

    void declaration::on_owned_elements(element_list_t& list) {
        if (_identifier != nullptr)
            list.emplace_back(_identifier);

        if (_assignment != nullptr)
            list.emplace_back(_assignment);
    }

    bool declaration::on_as_identifier(compiler::identifier*& value) const {
        value = _identifier;
        return true;
    }

}