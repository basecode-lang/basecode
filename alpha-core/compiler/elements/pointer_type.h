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

    class pointer_type : public compiler::type {
    public:
        static std::string name_for_pointer(compiler::type* base_type);

        pointer_type(
            compiler::block* parent_scope,
            compiler::type* base_type);

        compiler::type* base_type() const;

    protected:
        bool on_initialize(
            common::result& r,
            compiler::program* program) override;

        type_access_model_t on_access_model() const override;

    private:
        compiler::type* _base_type = nullptr;
    };

};

