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

    class boolean_literal : public element {
    public:
        boolean_literal(
            compiler::module* module,
            block* parent_scope,
            bool value);

        bool value() const;

        bool is_singleton() const override {
            return true;
        }

    protected:
        bool on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) override;

        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        bool on_is_constant() const override;

        bool on_as_bool(bool& value) const override;

        bool on_equals(const compiler::element& other) const override;

        bool on_not_equals(const compiler::element& other) const override;

    private:
        bool _value = false;
    };

};

