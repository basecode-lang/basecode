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

#include "unknown_type.h"

namespace basecode::compiler {

    unknown_type::unknown_type(
            compiler::module* module,
            block* parent_scope,
            compiler::symbol_element* symbol,
            compiler::element* expression) : compiler::type(module, parent_scope, element_type_t::unknown_type, symbol),
                                             _expression(expression) {
    }

    bool unknown_type::is_unknown_type() const {
        return true;
    }

    compiler::element* unknown_type::expression() const {
        return _expression;
    }

    void unknown_type::expression(compiler::element* value) {
        _expression = value;
    }

}