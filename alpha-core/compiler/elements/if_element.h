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
            block* parent_scope,
            element* predicate,
            element* true_branch,
            element* false_branch,
            bool is_else_if);

        element* predicate();

        element* true_branch();

        element* false_branch();

        bool is_else_if() const;

    protected:
        bool on_emit(compiler::session& session) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        bool _is_else_if = false;
        element* _predicate = nullptr;
        element* _true_branch = nullptr;
        element* _false_branch = nullptr;
    };

};

