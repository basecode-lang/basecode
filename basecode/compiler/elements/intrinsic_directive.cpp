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
#include "attribute.h"
#include "intrinsic.h"
#include "assignment.h"
#include "identifier.h"
#include "declaration.h"
#include "initializer.h"
#include "procedure_type.h"
#include "intrinsic_directive.h"

namespace basecode::compiler {

    intrinsic_directive::intrinsic_directive(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* expression) : directive(module, parent_scope),
                                             _expression(expression) {
    }

    directive_type_t intrinsic_directive::type() const {
        return directive_type_t::intrinsic_e;
    }

    bool intrinsic_directive::on_evaluate(compiler::session& session) {
        auto assignment = dynamic_cast<compiler::assignment*>(_expression);
        auto proc_decl = dynamic_cast<compiler::declaration*>(assignment->expressions()[0]);
        if (proc_decl == nullptr)
            return false;

        auto proc_type = proc_decl->identifier()->initializer()->procedure_type();

        std::string name;
        auto attr = find_attribute("name");
        if (attr != nullptr) {
            if (!attr->as_string(name)) {
                session.error(
                    module(),
                    "P004",
                    "unable to convert name.",
                    location());
                return false;
            }
        }

        if (name.empty()) {
            session.error(
                module(),
                "P005",
                "name attribute required for intrinsic directive.",
                location());
            return false;
        }

        if (!compiler::intrinsic::register_intrinsic_procedure_type(
                name,
                proc_type)) {
            session.error(
                module(),
                "X000",
                "unable to register intrinsic procedure type.",
                location());
            return false;
        }

        return true;
    }

    void intrinsic_directive::on_owned_elements(element_list_t& list) {
        list.emplace_back(_expression);
    }

}