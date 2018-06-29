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

#include <map>
#include <string>
#include <memory>
#include <unordered_map>
#include "type.h"
#include "element.h"
#include "initializer.h"

namespace basecode::compiler {

    class field : public element {
    public:
        field(
            element* parent,
            compiler::identifier* identifier);

        compiler::identifier* identifier();

    private:
        compiler::identifier* _identifier;
    };

};