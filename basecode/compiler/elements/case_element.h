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

    class case_element : public element {
    public:
        case_element(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::block* scope,
            compiler::element* expr);

        compiler::block* scope();

        compiler::element* expression();

    protected:
        bool on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        compiler::block* _scope = nullptr;
        compiler::element* _expr = nullptr;
    };

};

