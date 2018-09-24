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

#include "operator_base.h"
#include "element_types.h"

namespace basecode::compiler {

    class unary_operator : public operator_base {
    public:
        unary_operator(
            compiler::module* module,
            block* parent_scope,
            operator_type_t type,
            element* rhs);

        element* rhs();

        void rhs(compiler::element* element);

    protected:
        bool on_fold(
            compiler::session& session,
            fold_result_t& result) override;

        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        bool on_is_constant() const override;

        bool on_emit(compiler::session& session) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        element* _rhs = nullptr;
    };

};

