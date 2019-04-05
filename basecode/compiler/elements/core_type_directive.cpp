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
#include <compiler/scope_manager.h>
#include "block.h"
#include "program.h"
#include "attribute.h"
#include "identifier.h"
#include "assignment.h"
#include "declaration.h"
#include "initializer.h"
#include "type_directive.h"
#include "type_reference.h"
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
        switch (_expression->element_type()) {
            case element_type_t::directive: {
                auto directive = dynamic_cast<compiler::directive*>(_expression);
                if (directive == nullptr)
                    return false;
                switch (directive->type()) {
                    case directive_type_t::type: {
                        auto type_directive = dynamic_cast<compiler::type_directive*>(_expression);
                        auto expr = type_directive->expression();
                        if (expr != nullptr && expr->is_type()) {
                            auto type_ref = dynamic_cast<compiler::type_reference*>(expr);
                            session.scope_manager().add_type_to_scope(
                                type_ref->type(),
                                session.program().block());
                        }
                        break;
                    }
                    default: {
                        return false;
                    }
                }
                break;
            }
            case element_type_t::assignment: {
                auto assignment = dynamic_cast<compiler::assignment*>(_expression);
                if (assignment == nullptr)
                    return false;

                auto type_decl = dynamic_cast<compiler::declaration*>(assignment->expressions().front());
                if (type_decl == nullptr)
                    return false;

                session.program().block()->identifiers().add(type_decl->identifier());
                break;
            }
            default: {
                break;
            }
        }

        return true;
    }

    void core_type_directive::on_owned_elements(element_list_t& list) {
        list.emplace_back(_expression);
    }

}