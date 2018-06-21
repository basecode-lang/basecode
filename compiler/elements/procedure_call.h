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

#include "element.h"

namespace basecode::compiler {

    class procedure_call : public element {
    public:
        procedure_call(
            element* parent,
            compiler::type* procedure_type,
            element* expr);

        element* expression();

        compiler::type* procedure_type();

    private:
        element* _expression = nullptr;
        compiler::type* _procedure_type = nullptr;
    };

};

