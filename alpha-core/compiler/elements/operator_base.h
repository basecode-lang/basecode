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

    class operator_base : public element {
    public:
        operator_base(
            block* parent_scope,
            element_type_t element_type,
            operator_type_t operator_type);

        operator_type_t operator_type() const;

    private:
        operator_type_t _operator_type;
    };

};

