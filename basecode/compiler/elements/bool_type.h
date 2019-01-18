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
        bool_type(
            compiler::module* module,
            compiler::block* parent_scope);

    protected:
        bool on_type_check(compiler::type* other) override;

        number_class_t on_number_class() const override;

        bool on_initialize(compiler::session& session) override;
    };

};

