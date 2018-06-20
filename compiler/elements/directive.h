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

#include "block.h"

namespace basecode::compiler {

    class directive : public element {
    public:
        directive(
            block* parent,
            const std::string& name,
            element* expression);

        element* expression();

        std::string name() const;

    private:
        std::string _name;
        element* _expression = nullptr;
    };

};

