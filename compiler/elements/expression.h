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

#include <common/result.h>
#include "element.h"
#include "element_types.h"

namespace basecode::compiler {

    class expression : public element {
    public:
        explicit expression(element* root);

        ~expression() override;

    private:
        element* _root;
    };

};

