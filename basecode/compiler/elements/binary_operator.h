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
            block* parent_scope,
            operator_type_t type,
            element* lhs,
            element* rhs);

        element* lhs();

        element* rhs();

        void lhs(compiler::element* element);

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

        void emit_arithmetic_operator(compiler::session& session);

        void emit_relational_operator(compiler::session& session);

    private:
        element* _lhs = nullptr;
        element* _rhs = nullptr;
        vm::register_t _temp_reg {
            .size = vm::op_sizes::byte,
            .type = vm::register_type_t::integer
        };
    };

};

