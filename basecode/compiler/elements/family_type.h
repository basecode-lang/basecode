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

#include "type.h"

namespace basecode::compiler {

    class family_type : public compiler::type {
    public:
        family_type(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::symbol_element* symbol,
            compiler::type_reference_list_t types);

        const compiler::type_reference_list_t& types() const;

    protected:
        bool on_is_constant() const override;

        bool on_type_check(compiler::type* other) override;

        bool on_initialize(compiler::session& session) override;

    private:
        compiler::type_reference_list_t _types {};
    };

}

