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

    class directive : public element {
    public:
        directive(const std::string& name, expression* rhs);

        ~directive() override;

        expression* rhs();

        std::string name() const;

    private:
        std::string _name;
        expression* _rhs = nullptr;
    };

};

