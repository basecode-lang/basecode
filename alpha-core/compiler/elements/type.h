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
#include <unordered_map>
#include "element.h"

namespace basecode::compiler {

    class type : public element {
    public:
        type(
            element* parent,
            element_type_t type,
            const std::string& name);

        std::string name() const;

        void name(const std::string& value);

    private:
        std::string _name;
    };

};