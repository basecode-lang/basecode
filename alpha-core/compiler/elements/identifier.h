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

    enum class identifier_usage_t : uint8_t {
        heap = 1,
        stack
    };

    class identifier : public element {
    public:
        identifier(
            block* parent_scope,
            compiler::symbol_element* name,
            compiler::initializer* initializer);

        compiler::type* type();

        bool inferred_type() const;

        void type(compiler::type* t);

        void inferred_type(bool value);

        identifier_usage_t usage() const;

        void usage(identifier_usage_t value);

        compiler::initializer* initializer();

        compiler::symbol_element* symbol() const;

        void initializer(compiler::initializer* value);

    protected:
        bool on_emit(
            common::result& r,
            emit_context_t& context) override;

        bool on_is_constant() const override;

        bool on_as_bool(bool& value) const override;

        bool on_as_float(double& value) const override;

        bool on_as_integer(uint64_t& value) const override;

        bool on_as_string(std::string& value) const override;

        void on_owned_elements(element_list_t& list) override;

        compiler::type* on_infer_type(const compiler::program* program) override;

    private:
        void emit_stack_based_load(
            vm::instruction_block* instruction_block);

    private:
        bool _inferred_type = false;
        compiler::type* _type = nullptr;
        compiler::symbol_element* _symbol;
        compiler::initializer* _initializer;
        identifier_usage_t _usage = identifier_usage_t::heap;
    };

};