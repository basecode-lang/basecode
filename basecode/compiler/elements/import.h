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

    class import : public element {
    public:
        import(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* expr,
            compiler::element* from_expr,
            compiler::module* imported_module);

        compiler::element* expression();

        compiler::module* imported_module();

        compiler::element* from_expression();

    protected:
        void on_owned_elements(element_list_t& list) override;

    private:
        compiler::element* _expression = nullptr;
        compiler::module* _imported_module = nullptr;
        compiler::element* _from_expression = nullptr;
    };

};

