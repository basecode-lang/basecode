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

#include "operator_base.h"
#include "element_types.h"

namespace basecode::compiler {

    class unary_operator : public operator_base {
    public:
        explicit unary_operator(
            element* parent,
            operator_type_t type,
            element* rhs);

        element* rhs();

    protected:
        compiler::type* on_infer_type(const compiler::program* program) override;

    private:
        element* _rhs = nullptr;
    };

};

