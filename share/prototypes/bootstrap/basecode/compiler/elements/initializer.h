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

    class initializer : public element {
    public:
        initializer(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* expr);

        bool is_nil() const;

        compiler::element* expression();

        void expression(element* value);

        compiler::procedure_type* procedure_type();

    protected:
        bool on_fold(
            compiler::session& session,
            fold_result_t& result) override;

        bool on_infer_type(
            compiler::session& program,
            infer_type_result_t& result) override;

        bool on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) override;

        bool on_is_constant() const override;

        bool on_as_bool(bool& value) const override;

        bool on_as_float(double& value) const override;

        bool on_as_integer(uint64_t& value) const override;

        bool on_as_string(std::string& value) const override;

        void on_owned_elements(element_list_t& list) override;

        bool on_as_rune(common::rune_t& value) const override;

    private:
        compiler::element* _expr = nullptr;
    };

};

