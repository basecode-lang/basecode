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

#include "directive.h"
#include "if_directive.h"
#include "run_directive.h"
#include "type_directive.h"
#include "assert_directive.h"
#include "foreign_directive.h"
#include "assembly_directive.h"
#include "core_type_directive.h"
#include "intrinsic_directive.h"

namespace basecode::compiler {

    directive* directive::directive_for_type(
            compiler::module* module,
            compiler::block* parent_scope,
            directive_type_t type,
            const common::source_location& location,
            const element_list_t& params) {
        if (params.empty())
            return nullptr;

        switch (type) {
            case directive_type_t::run: {
                auto instance = new run_directive(module, parent_scope, params[0]);
                params[0]->parent_element(instance);
                instance->location(location);
                return instance;
            }
            case directive_type_t::if_e: {
                compiler::element* lhs = nullptr;
                if (!params.empty())
                    lhs = params[0];
                compiler::element* rhs = nullptr;
                if (params.size() > 1)
                    rhs = params[1];
                compiler::element* body = nullptr;
                if (params.size() > 2)
                    body = params[2];
                auto instance = new if_directive(
                    module,
                    parent_scope,
                    lhs,
                    rhs,
                    body);
                if (lhs != nullptr)
                    lhs->parent_element(instance);
                if (rhs != nullptr)
                    rhs->parent_element(instance);
                if (body != nullptr)
                    body->parent_element(instance);
                instance->location(location);
                return instance;
            }
            case directive_type_t::eval: {
                break;
            }
            case directive_type_t::type: {
                auto instance = new type_directive(module, parent_scope, params[0]);
                params[0]->parent_element(instance);
                instance->location(location);
                return instance;
            }
            case directive_type_t::assert: {
                auto instance = new assert_directive(module, parent_scope, params[0]);
                params[0]->parent_element(instance);
                instance->location(location);
                return instance;
            }
            case directive_type_t::foreign: {
                auto instance = new foreign_directive(module, parent_scope, params[0]);
                params[0]->parent_element(instance);
                instance->location(location);
                return instance;
            }
            case directive_type_t::assembly: {
                auto instance = new assembly_directive(module, parent_scope, params[0]);
                params[0]->parent_element(instance);
                instance->location(location);
                return instance;
            }
            case directive_type_t::core_type: {
                auto instance = new core_type_directive(module, parent_scope, params[0]);
                params[0]->parent_element(instance);
                instance->location(location);
                return instance;
            }
            case directive_type_t::intrinsic_e: {
                auto instance = new intrinsic_directive(module, parent_scope, params[0]);
                params[0]->parent_element(instance);
                instance->location(location);
                return instance;
            }
            default: {
                break;
            }
        }
        
        return nullptr;
    }

    ///////////////////////////////////////////////////////////////////////////

    directive::directive(
            compiler::module* module,
            compiler::block* parent_scope) : element(module, parent_scope, element_type_t::directive) {
    }

    directive_type_t directive::type() const {
        return directive_type_t::unknown;
    }

    bool directive::execute(compiler::session& session) {
        return on_execute(session);
    }

    bool directive::evaluate(compiler::session& session) {
        return on_evaluate(session);
    }

    bool directive::on_execute(compiler::session& session) {
        return true;
    }

    bool directive::on_evaluate(compiler::session& session) {
        return true;
    }

}