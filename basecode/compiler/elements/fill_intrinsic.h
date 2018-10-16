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

    class fill_intrinsic : public intrinsic {
    public:
        fill_intrinsic(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::argument_list* args);

        std::string name() const override;

    protected:
        bool on_emit(compiler::session& session) override;
    };

};

