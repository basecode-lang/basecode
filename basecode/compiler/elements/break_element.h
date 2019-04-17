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

    class break_element : public element {
    public:
        break_element(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* label);

        compiler::element* label();

    protected:
        bool on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) override;

        compiler::element* on_clone(
            compiler::session& session,
            compiler::block* new_scope) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        compiler::element* _label = nullptr;
    };

}

