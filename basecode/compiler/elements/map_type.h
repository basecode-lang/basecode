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

#include "composite_type.h"

namespace basecode::compiler {

    class map_type  : public compiler::composite_type {
    public:
        static std::string name_for_map(
            compiler::type_reference* key_type,
            compiler::type_reference* value_type);

        map_type(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::block* scope,
            compiler::type_reference* key_type,
            compiler::type_reference* value_type);

        compiler::type_reference* key_type();

        compiler::type_reference* value_type();

        std::string name(const std::string& alias = "") const override;

    protected:
        type_access_model_t on_access_model() const override;

        bool on_initialize(compiler::session& session) override;

    private:
        compiler::type_reference* _key_type = nullptr;
        compiler::type_reference* _value_type = nullptr;
    };

};