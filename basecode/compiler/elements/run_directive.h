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

    class run_directive : public directive {
    public:
        run_directive(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* expression);

        compiler::element* expression() const;

        directive_type_t type() const override;

    protected:
        bool on_execute(compiler::session& session) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        compiler::element* _expression = nullptr;
    };

}

