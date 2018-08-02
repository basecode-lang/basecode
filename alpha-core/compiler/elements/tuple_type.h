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

#include "composite_type.h"

namespace basecode::compiler {

    class tuple_type : public compiler::composite_type {
    public:
        tuple_type(
            compiler::block* parent_scope,
            compiler::block* scope);

    protected:
        bool on_initialize(
            common::result& r,
            compiler::program* program) override;

        type_access_model_t on_access_model() const override;
    };

};

