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
#include "block.h"
#include "program.h"
#include "attribute.h"
#include "identifier.h"
#include "assignment.h"
#include "declaration.h"
#include "initializer.h"
#include "core_type_directive.h"

namespace basecode::compiler {

    core_type_directive::core_type_directive(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* expression) : directive(module, parent_scope),
                                             _expression(expression) {
    }

    directive_type_t core_type_directive::type() const {
        return directive_type_t::core_type;
    }

    bool core_type_directive::on_evaluate(compiler::session& session) {
        auto assignment = dynamic_cast<compiler::assignment*>(_expression);
        if (assignment == nullptr)
            return false;

        auto type_decl = dynamic_cast<compiler::declaration*>(assignment->expressions().front());
        if (type_decl == nullptr)
            return false;

        session.program().block()->identifiers().add(type_decl->identifier());

        return true;
    }

    void core_type_directive::on_owned_elements(element_list_t& list) {
        list.emplace_back(_expression);
    }

}