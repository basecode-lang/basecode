// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#pragma once

#include "element.h"

namespace basecode::compiler {

    class identifier : public element {
    public:
        identifier(
            element* parent,
            const std::string& name,
            compiler::initializer* initializer);

        bool constant() const;

        compiler::type* type();

        std::string name() const;

        bool stack_based() const;

        void constant(bool value);

        bool inferred_type() const;

        void stack_based(bool value);

        void type(compiler::type* t);

        void inferred_type(bool value);

        compiler::initializer* initializer();

    protected:
        bool on_emit(
            common::result& r,
            vm::assembler& assembler,
            const emit_context_t& context) override;

        bool on_as_bool(bool& value) const override;

        bool on_as_float(double& value) const override;

        bool on_as_integer(uint64_t& value) const override;

        bool on_as_string(std::string& value) const override;

    private:
        std::string _name;
        bool _constant = false;
        bool _stack_based = false;
        bool _inferred_type = false;
        compiler::type* _type = nullptr;
        compiler::initializer* _initializer;
    };

};