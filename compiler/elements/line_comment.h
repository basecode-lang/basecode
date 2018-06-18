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

    class line_comment : public element {
    public:
        explicit line_comment(const std::string& value);

        std::string value() const {
            return _value;
        }

    private:
        std::string _value;
    };

};

