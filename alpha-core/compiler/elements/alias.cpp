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

#include "alias.h"

namespace basecode::compiler {

    alias::alias(
            compiler::module* module,
            block* parent_scope,
            element* expr) : element(module, parent_scope, element_type_t::alias_type),
                             _expression(expr) {
    }

    element* alias::expression() {
        return _expression;
    }

    void alias::on_owned_elements(element_list_t& list) {
        if (_expression != nullptr)
            list.emplace_back(_expression);
    }

};