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

#include "directive.h"

namespace basecode::compiler {

    directive::directive(
            block* parent,
            const std::string& name,
            element* expression) : element(parent, element_type_t::directive),
                                   _name(name),
                                   _expression(expression) {
    }

    element* directive::expression() {
        return _expression;
    }

    std::string directive::name() const {
        return _name;
    }

};