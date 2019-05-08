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

#include "directive.h"

namespace basecode::compiler {

    class inline_directive : public directive {
    public:
        inline_directive(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* expression);

        compiler::element* expression();

        bool is_valid_data() const override;

        directive_type_t type() const override;

    protected:
        bool on_fold(
            compiler::session& session,
            fold_result_t& result) override;

        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        bool on_is_constant() const override;

        compiler::element* on_clone(
            compiler::session& session,
            compiler::block* new_scope) override;

        bool on_evaluate(compiler::session& session) override;

    private:
        compiler::element* _expression = nullptr;
    };

}

