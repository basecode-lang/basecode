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

    class namespace_type : public compiler::type {
    public:
        explicit namespace_type(block* parent_scope);

    protected:
        bool on_type_check(compiler::type* other) override;

        bool on_initialize(compiler::session& session) override;
    };

};

