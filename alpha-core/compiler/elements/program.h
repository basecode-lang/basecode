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

#include <functional>
#include <parser/ast.h>
#include <common/id_pool.h>
#include <common/source_file.h>
#include <compiler/compiler_types.h>
#include "element.h"

namespace basecode::compiler {

    class program : public element {
    public:
        program();

        ~program() override;

        compiler::block* block();

        void block(compiler::block* value);

    private:
        bool on_emit(compiler::session& session) override;

    private:
        compiler::block* _block = nullptr;
    };

};
