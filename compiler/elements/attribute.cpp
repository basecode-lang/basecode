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

#include "attribute.h"

namespace basecode::compiler {

    attribute::attribute(
        const std::string& name,
        expression* rhs) : _name(name),
                           _rhs(rhs) {
    }

    attribute::~attribute() {
    }

};