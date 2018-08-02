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

#include "type.h"

namespace basecode::compiler {

    class bool_type : public compiler::type {
    public:
        explicit bool_type(block* parent_scope);

    protected:
        bool on_initialize(
            common::result& r,
            compiler::program* program) override;

        type_number_class_t on_number_class() const override;

        type_access_model_t on_access_model() const override;
    };

};

