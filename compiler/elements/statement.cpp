// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#include "statement.h"

namespace basecode::compiler {

    statement::statement(
            element* parent,
            element* expr) : element(parent, element_type_t::statement),
                             _expr(expr) {
    }

    element* statement::expr() {
        return _expr;
    }

    label_list_t& statement::labels() {
        return _labels;
    }

};