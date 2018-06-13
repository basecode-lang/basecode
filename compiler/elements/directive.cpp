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

    directive::directive(const std::string& name, expression* rhs) : _name(name),
                                                                     _rhs(rhs) {
    }

    directive::~directive() {
    }

    std::string directive::name() const {
        return _name;
    }

    compiler::expression* directive::rhs() {
        return _rhs;
    }

};