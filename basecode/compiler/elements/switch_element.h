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

    class switch_element : public element {
    public:
        switch_element(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::block* scope,
            compiler::element* expression);

        compiler::block* scope();

        compiler::element* expression();

    protected:
        compiler::element* on_clone(
            compiler::session& session,
            compiler::block* new_scope) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        compiler::block* _scope = nullptr;
        compiler::element* _expr = nullptr;
    };

}

