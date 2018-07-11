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

    class string_literal : public element {
    public:
        string_literal(
            block* parent_scope,
            const std::string& value);

        inline std::string value() const {
            return _value;
        }

    protected:
        bool on_emit(
            common::result& r,
            emit_context_t& context) override;

        bool on_is_constant() const override;

        bool on_as_string(std::string& value) const override;

        compiler::type* on_infer_type(const compiler::program* program) override;

    private:
        std::string _value;
    };

};

