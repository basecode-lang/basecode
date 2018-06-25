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

    class statement : public element {
    public:
        statement(
            element* parent,
            element* expr);

        element* expression();

        label_list_t& labels();

    private:
        label_list_t _labels {};
        compiler::element* _expression = nullptr;
    };

};

