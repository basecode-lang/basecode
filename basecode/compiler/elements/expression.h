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

    class expression : public element {
    public:
        expression(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* root);

        compiler::element* root();

        void root(compiler::element* value);

    protected:
        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        bool on_is_constant() const override;

        bool on_as_bool(bool& value) const override;

        bool on_as_float(double& value) const override;

        bool on_emit(compiler::session& session) override;

        bool on_as_integer(uint64_t& value) const override;

        void on_owned_elements(element_list_t& list) override;

    private:
        compiler::element* _root = nullptr;
    };

};

