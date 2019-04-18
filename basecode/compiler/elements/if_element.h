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

    class if_element : public element {
    public:
        if_element(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* predicate,
            compiler::element* true_branch,
            compiler::element* false_branch,
            bool is_else_if);

        bool is_else_if() const;

        compiler::element* predicate();

        compiler::element* true_branch();

        compiler::element* false_branch();

        void predicate(compiler::element* value);

    protected:
        bool on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) override;

        compiler::element* on_clone(
            compiler::session& session,
            compiler::block* new_scope) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        bool _is_else_if = false;
        compiler::element* _predicate = nullptr;
        compiler::element* _true_branch = nullptr;
        compiler::element* _false_branch = nullptr;
    };

}

