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

#include "intrinsic.h"

namespace basecode::compiler {

    class free_intrinsic : public intrinsic {
    public:
        free_intrinsic(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::argument_list* args);

    protected:
        bool on_emit(compiler::session& session);
    };

};

