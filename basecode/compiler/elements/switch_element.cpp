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
#include "label.h"
#include "block.h"
#include "switch_element.h"

namespace basecode::compiler {

    switch_element::switch_element(
        compiler::module* module,
        compiler::block* parent_scope,
        compiler::block* scope,
        compiler::element* expression) : element(module, parent_scope, element_type_t::switch_e),
                                         _scope(scope),
                                         _expr(expression) {
    }

    compiler::block* switch_element::scope() {
        return _scope;
    }

    compiler::element* switch_element::expression() {
        return _expr;
    }

    void switch_element::on_owned_elements(element_list_t& list) {
        if (_scope != nullptr)
            list.emplace_back(_scope);

        if (_expr != nullptr)
            list.emplace_back(_expr);
    }

};