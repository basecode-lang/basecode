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

#include "type.h"

namespace basecode::compiler {

    class boolean_literal : public type {
    public:
        boolean_literal(
            element* parent,
            const std::string& name,
            bool value);

        bool value() const {
            return _value;
        }

    private:
        bool _value = false;
    };

};

