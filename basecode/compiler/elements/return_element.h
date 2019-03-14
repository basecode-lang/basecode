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

    class return_element : public element {
    public:
        return_element(
            compiler::module* module,
            block* parent_scope);

        compiler::field* field();

        element_list_t& expressions();

        void field(compiler::field* value);

    protected:
        bool on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        element_list_t _expressions {};
        compiler::field* _field = nullptr;
    };

}

