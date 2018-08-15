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

#include <vm/instruction_block.h>
#include "operator_base.h"
#include "element_types.h"

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

    protected:
        void emit_arithmetic_operator(
            compiler::session& session,
            vm::instruction_block* instruction_block);

        void emit_relational_operator(
            compiler::session& session,
            vm::instruction_block* instruction_block);

        bool on_is_constant() const override;

        bool on_emit(compiler::session& session) override;

        void on_owned_elements(element_list_t& list) override;

        element* on_fold(compiler::session& session) override;

        compiler::type* on_infer_type(const compiler::program* program) override;

    private:
        element* _lhs = nullptr;
        element* _rhs = nullptr;
    };

};

