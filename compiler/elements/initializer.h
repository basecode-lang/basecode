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

#pragma once

#include "expression.h"

namespace basecode::compiler {

    class initializer : public expression {
    public:
        initializer(
            element* parent,
            element* expr);
    };

};

