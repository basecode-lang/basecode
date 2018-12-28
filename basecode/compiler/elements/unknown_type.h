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

    class unknown_type : public compiler::type {
    public:
        unknown_type(
            compiler::module* module,
            block* parent_scope,
            compiler::symbol_element* symbol,
            compiler::element* expression);

        bool is_unknown_type() const override;

        compiler::element* expression() const;

        void expression(compiler::element* value);

    private:
        compiler::element* _expression = nullptr;
    };

};

