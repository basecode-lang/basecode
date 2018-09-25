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

#include <compiler/session.h>
#include "composite_type.h"

namespace basecode::compiler {

    class type_info : public compiler::composite_type {
    public:
        type_info(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::block* scope);

    protected:
        bool on_emit_initializer(
            compiler::session& session,
            compiler::identifier* var) override;

        type_access_model_t on_access_model() const override;

        bool on_initialize(compiler::session& session) override;
    };

};

