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

#include <string>
#include "element.h"
#include "expression.h"

namespace basecode::compiler {

    class attribute : public element {
    public:
        attribute(
            const std::string& name,
            expression* rhs);

        ~attribute() override;

    private:
        std::string _name;
        expression* _rhs = nullptr;
    };

};

