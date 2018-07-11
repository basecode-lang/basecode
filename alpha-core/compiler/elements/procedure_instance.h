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

    class procedure_instance : public element {
    public:
        procedure_instance(
            block* parent_scope,
            compiler::type* procedure_type,
            block* scope);

        block* scope();

        compiler::type* procedure_type();

    protected:
        bool on_emit(
            common::result& r,
            emit_context_t& context);

    private:
        block* _scope = nullptr;
        compiler::type* _procedure_type = nullptr;
    };

};

