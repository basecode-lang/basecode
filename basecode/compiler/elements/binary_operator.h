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

#include <vm/vm_types.h>
#include "operator_base.h"

namespace basecode::compiler {

    class binary_operator : public operator_base {
    public:
        binary_operator(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::operator_type_t type,
            compiler::element* lhs,
            compiler::element* rhs);

        compiler::element* lhs();

        compiler::element* rhs();

        void lhs(compiler::element* element);

        void rhs(compiler::element* element);

        std::string label_name() const override;

    protected:
        bool on_fold(
            compiler::session& session,
            fold_result_t& result) override;

        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        bool on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) override;

        compiler::element* on_clone(
            compiler::session& session,
            compiler::block* new_scope) override;

        bool on_is_constant() const override;

        bool on_as_bool(bool& value) const override;

        bool on_as_float(double& value) const override;

        bool on_as_integer(uint64_t& value) const override;

        void on_owned_elements(element_list_t& list) override;

    private:
        compiler::element* _lhs = nullptr;
        compiler::element* _rhs = nullptr;
    };

}

