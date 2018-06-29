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

    class integer_literal : public element {
    public:
        integer_literal(
            element* parent,
            uint64_t value);

        uint64_t value() const;

    protected:
        compiler::type* on_infer_type(const compiler::program* program) override;

    private:
        uint64_t _value;
    };

};

