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
#include "run_directive.h"

namespace basecode::compiler {

    run_directive::run_directive(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* expression) : directive(module, parent_scope),
                                             _expression(expression) {
    }

    directive_type_t run_directive::type() const {
        return directive_type_t::run;
    }

    compiler::element* run_directive::expression() const {
        return _expression;
    }

    bool run_directive::on_execute(compiler::session& session) {
        session.enable_run();
        return true;
    }

    void run_directive::on_owned_elements(element_list_t& list) {
        list.emplace_back(_expression);
    }

}