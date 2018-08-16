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

        void disassemble(
            compiler::session& session,
            FILE* file);

        compiler::block* block();

        compiler::module* compile_module(
            compiler::session& session,
            common::source_file* source_file);

        bool compile(compiler::session& session);

    private:
        bool type_check(compiler::session& session);

        bool on_emit(compiler::session& session) override;

        bool resolve_unknown_types(compiler::session& session);

        void initialize_core_types(compiler::session& session);

        bool resolve_unknown_identifiers(compiler::session& session);

    private:
        compiler::block* _block = nullptr;
    };

};
