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

    class identifier_reference : public element {
    public:
        identifier_reference(
            compiler::module* module,
            block* parent_scope,
            const qualified_symbol_t& symbol,
            compiler::identifier* identifier);

        bool resolved() const;

        offset_result_t field_offset();

        compiler::identifier* identifier();

        std::string label_name() const override;

        const qualified_symbol_t& symbol() const;

        void identifier(compiler::identifier* value);

    protected:
        bool on_fold(
            compiler::session& session,
            fold_result_t& result) override;

        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        compiler::element* on_clone(
            compiler::session& session,
            compiler::block* new_scope) override;

        bool on_is_constant() const override;

        bool on_as_bool(bool& value) const override;

        bool on_as_float(double& value) const override;

        bool on_as_integer(uint64_t& value) const override;

        bool on_equals(const element& other) const override;

        bool on_as_string(std::string& value) const override;

        bool on_as_rune(common::rune_t& value) const override;

        bool on_less_than(const element& other) const override;

        bool on_not_equals(const element& other) const override;

        bool on_greater_than(const element& other) const override;

        bool on_less_than_or_equal(const element& other) const override;

        bool on_greater_than_or_equal(const element& other) const override;

        bool on_as_identifier(compiler::identifier*& value) const override;

    private:
        static void evaluate_member_access_expression(
            compiler::element* e,
            binary_operator_stack_t& bin_op_stack,
            identifier_reference_stack_t& ref_stack);

    private:
        qualified_symbol_t _symbol;
        compiler::identifier* _identifier = nullptr;
    };

}

