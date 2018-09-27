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

#include <compiler/session.h>
#include "composite_type.h"

namespace basecode::compiler {

    class array_type : public compiler::composite_type {
    public:
        static std::string name_for_array(
            compiler::type* entry_type,
            const element_list_t& subscripts);

        array_type(
            compiler::module* module,
            block* parent_scope,
            compiler::block* scope,
            compiler::type_reference* entry_type,
            const element_list_t& subscripts);

        const element_list_t& subscripts() const;

        compiler::type_reference* entry_type_ref();

        std::string name(const std::string& alias = "") const override;

    protected:
        type_access_model_t on_access_model() const override;

        void on_owned_elements(element_list_t& list) override;

        bool on_initialize(compiler::session& session) override;

    private:
        element_list_t _subscripts {};
        compiler::type_reference* _entry_type_ref = nullptr;
    };

};

