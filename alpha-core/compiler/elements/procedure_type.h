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
#include "field.h"

namespace basecode::compiler {

    class procedure_type : public compiler::type {
    public:
        procedure_type(
            block* parent_scope,
            compiler::block* scope,
            compiler::symbol_element* symbol);

        field_map_t& returns();

        bool is_foreign() const;

        compiler::block* scope();

        field_map_t& parameters();

        void is_foreign(bool value);

        type_map_t& type_parameters();

        uint64_t foreign_address() const;

        void foreign_address(uint64_t value);

        procedure_instance_list_t& instances();

    protected:
        bool on_emit(
            common::result& r,
            emit_context_t& context) override;

        bool on_initialize(
            common::result& r,
            compiler::program* program) override;

        bool on_is_constant() const override;

        type_access_model_t on_access_model() const override;

        void on_owned_elements(element_list_t& list) override;

    private:
        field_map_t _returns {};
        bool _is_foreign = false;
        field_map_t _parameters {};
        uint64_t _foreign_address = 0;
        type_map_t _type_parameters {};
        compiler::block* _scope = nullptr;
        procedure_instance_list_t _instances {};
    };

};

