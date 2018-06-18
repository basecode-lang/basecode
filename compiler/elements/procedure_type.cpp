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

#include "procedure_type.h"
#include "field.h"

namespace basecode::compiler {

    procedure_type::procedure_type(
        element* parent,
        const std::string& name) : type(parent, name),
                                   _parameters(this),
                                   _returns(this) {
    }

    procedure_type::~procedure_type() {
    }

};