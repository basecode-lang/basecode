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
#include "type.h"
#include "block.h"
#include "label.h"
#include "raw_block.h"
#include "type_reference.h"
#include "assembly_directive.h"

namespace basecode::compiler {

    assembly_directive::assembly_directive(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* expression) : directive(module, parent_scope, "assembly"),
                                             _expression(expression) {
    }

    compiler::element* assembly_directive::expression() const {
        return _expression;
    }

    void assembly_directive::on_owned_elements(element_list_t& list) {
        list.emplace_back(_expression);
    }

    bool assembly_directive::on_evaluate(compiler::session& session) {
        auto is_valid = _expression != nullptr
            && _expression->element_type() == element_type_t::raw_block;
        if (!is_valid) {
            session.error(
                module(),
                "P004",
                "#assembly expects a valid raw block expression.",
                location());
            return false;
        }

        return true;
    }

};