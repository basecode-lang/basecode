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

    class spread_operator : public compiler::element {
    public:
        spread_operator(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* expr);

        compiler::element* expr();

    protected:
        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        compiler::element* on_clone(
            compiler::session& session,
            compiler::block* new_scope) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        compiler::element* _expr = nullptr;
    };

}

