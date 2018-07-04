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

    class expression : public element {
    public:
        expression(
            element* parent,
            element* root);

        element* root();

    protected:
        bool on_is_constant() const override;

        compiler::type* on_infer_type(const compiler::program* program) override;

    private:
        element* _root = nullptr;
    };

};

