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

    class integer_literal : public element {
    public:
        integer_literal(
            compiler::module* module,
            block* parent_scope,
            uint64_t value,
            compiler::type_reference* type_ref = nullptr,
            bool is_signed = false);

        bool is_signed() const;

        bool is_sign_bit_high() const;

        uint64_t value() const;

        void value(uint64_t v);

    protected:
        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        bool on_is_constant() const override;

        bool on_as_integer(uint64_t& value) const override;

        bool on_equals(const compiler::element& other) const override;

        uint64_t on_add(const compiler::element& other) const override;

        bool on_less_than(const compiler::element& other) const override;

        bool on_not_equals(const compiler::element& other) const override;

        uint64_t on_subtract(const compiler::element& other) const override;

        uint64_t on_multiply(const compiler::element& other) const override;

        bool on_greater_than(const compiler::element& other) const override;

        bool on_less_than_or_equal(const compiler::element& other) const override;

        bool on_greater_than_or_equal(const compiler::element& other) const override;

    private:
        uint64_t _value;
        bool _is_signed = false;
        compiler::type_reference* _type_ref = nullptr;
    };

}

