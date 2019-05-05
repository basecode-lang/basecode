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

#include "composite_type.h"

namespace basecode::compiler {

    class language_type : public compiler::composite_type {
    public:
        static std::string name_for_language();

        language_type(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::block* scope,
            compiler::raw_block* grammar,
            compiler::raw_block* translator,
            compiler::symbol_element* symbol);

        compiler::raw_block* grammar();

        compiler::raw_block* translator();

    protected:
        bool on_type_check(
            compiler::type* other,
            const type_check_options_t& options) override;

        bool on_initialize(compiler::session& session) override;

    private:
        compiler::raw_block* _grammar = nullptr;
        compiler::raw_block* _translator = nullptr;
    };

}

