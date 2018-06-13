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

#include "element_types.h"
#include "block.h"

namespace basecode::compiler {

    class namespace_element : public block {
    public:
        namespace_element(
            block* parent,
            const std::string& name);

        ~namespace_element() override;

        std::string name() const;

    private:
        std::string _name;
    };

};

