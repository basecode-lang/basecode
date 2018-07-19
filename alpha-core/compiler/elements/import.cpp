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

#include "import.h"

namespace basecode::compiler {

    import::import(
        block* parent_scope,
        element* expr) : element(parent_scope, element_type_t::import_e),
                         _expression(expr) {
    }

    element* import::expression() {
        return _expression;
    }

    void import::on_owned_elements(element_list_t& list) {
        if (_expression != nullptr)
            list.emplace_back(_expression);
    }

};