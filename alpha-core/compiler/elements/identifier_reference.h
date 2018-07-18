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

    class identifier_reference  : public element {
    public:
        identifier_reference(
            block* parent_scope,
            const qualified_symbol_t& symbol,
            compiler::identifier* identifier);

        bool resolved() const;

        compiler::identifier* identifier();

        const qualified_symbol_t& symbol() const;

        void identifier(compiler::identifier* value);

    protected:
        bool on_emit(
            common::result& r,
            emit_context_t& context) override;

        bool on_is_constant() const override;

    private:
        qualified_symbol_t _symbol;
        compiler::identifier* _identifier = nullptr;
    };

};

