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
        const std::string& name,
        block* parent) : block(parent),
                         _name(name) {
    }

    std::string directive::name() const {
        return _name;
    }

};