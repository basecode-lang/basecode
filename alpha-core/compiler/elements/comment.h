// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include "element.h"

namespace basecode::compiler {

    class comment : public element {
    public:
        comment(
            element* parent,
            comment_type_t type,
            const std::string& value);

        std::string value() const;

        comment_type_t type() const;

    private:
        std::string _value;
        comment_type_t _type;
    };

};

