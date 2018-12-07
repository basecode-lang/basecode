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

    class argument_pair : public element {
    public:
        argument_pair(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* lhs,
            compiler::element* rhs);

        compiler::element* lhs();

        compiler::element* rhs();

        void rhs(compiler::element* value);

    protected:
        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        bool on_is_constant() const override;

        void on_owned_elements(element_list_t& list) override;

    private:
        compiler::element* _lhs = nullptr;
        compiler::element* _rhs = nullptr;
    };

};

