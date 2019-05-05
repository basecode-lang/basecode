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
#include "type.h"
#include "block.h"
#include "label.h"
#include "raw_block.h"
#include "type_reference.h"
#include "language_directive.h"

namespace basecode::compiler {

    language_directive::language_directive(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* language,
            compiler::element* expression) : directive(module, parent_scope),
                                             _language(language),
                                             _expression(expression) {
    }

    compiler::element* language_directive::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        auto expr = _expression != nullptr ?
                    _expression->clone<compiler::element>(session, new_scope) :
                    nullptr;
        auto directive = session
            .builder()
            .make_directive(new_scope, type(), location(), {expr});
        return directive;
    }

    directive_type_t language_directive::type() const {
        return directive_type_t::language;
    }

    compiler::element* language_directive::language() const {
        return _language;
    }

    compiler::element* language_directive::expression() const {
        return _expression;
    }

    void language_directive::on_owned_elements(element_list_t& list) {
        list.emplace_back(_expression);
    }

    bool language_directive::on_evaluate(compiler::session& session) {
        auto is_valid = _expression != nullptr
                        && _expression->element_type() == element_type_t::raw_block;
        if (!is_valid) {
            session.error(
                module(),
                "P004",
                "#language expects a valid raw block expression.",
                location());
            return false;
        }

        return true;
    }

}