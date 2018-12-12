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

#include <compiler/compiler_types.h>
#include "element.h"

namespace basecode::compiler {

    class identifier : public element {
    public:
        identifier(
            compiler::module* module,
            block* parent_scope,
            compiler::symbol_element* name,
            compiler::initializer* initializer);

        int64_t offset() const {
            return _offset;
        }

        compiler::field* field();

        bool inferred_type() const;

        void offset(int64_t value) {
            _offset = value;
        }

        void inferred_type(bool value);

        identifier_usage_t usage() const;

        void field(compiler::field* value);

        compiler::type_reference* type_ref();

        void usage(identifier_usage_t value);

        compiler::initializer* initializer();

        std::string label_name() const override;

        compiler::symbol_element* symbol() const;

        void type_ref(compiler::type_reference* t);

        void initializer(compiler::initializer* value);

    protected:
        bool on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) override;

        bool on_fold(
            compiler::session& session,
            fold_result_t& result) override;

        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        bool on_is_constant() const override;

        bool on_as_bool(bool& value) const override;

        bool on_as_float(double& value) const override;

        bool on_as_integer(uint64_t& value) const override;

        bool on_as_string(std::string& value) const override;

        void on_owned_elements(element_list_t& list) override;

        bool on_as_rune(common::rune_t& value) const override;

    private:
        int64_t _offset = 0;
        bool _inferred_type = false;
        compiler::field* _field = nullptr;
        compiler::symbol_element* _symbol;
        compiler::initializer* _initializer;
        compiler::type_reference* _type_ref = nullptr;
        identifier_usage_t _usage = identifier_usage_t::heap;
    };

};