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
        element* parent,
        element* expr) : element(parent, element_type_t::import_e),
                         _expression(expr) {
    }

    element* import::expression() {
        return _expression;
    }

};