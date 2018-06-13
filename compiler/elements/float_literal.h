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

    class float_literal : public element {
    public:
        explicit float_literal(double value);

        ~float_literal() override;

        inline double value() const {
            return _value;
        }

    private:
        double _value;
    };

};

