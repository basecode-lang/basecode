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

    class if_element : public element {
    public:
        if_element(
            element* parent,
            element* predicate,
            element* true_branch,
            element* false_branch);

        element* predicate();

        element* true_branch();

        element* false_branch();

    private:
        element* _predicate = nullptr;
        element* _true_branch = nullptr;
        element* _false_branch = nullptr;
    };

};

