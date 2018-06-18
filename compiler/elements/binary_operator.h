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

    class binary_operator : public operator_base {
    public:
        binary_operator(
            element* parent,
            operator_type_t type,
            expression* lhs,
            expression* rhs);

        ~binary_operator() override;

        expression* lhs();

        expression* rhs();

    private:
        expression* _lhs = nullptr;
        expression* _rhs = nullptr;
    };

};

