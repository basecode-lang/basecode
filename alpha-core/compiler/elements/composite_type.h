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

    class composite_type : public compiler::type {
    public:
        composite_type(
            block* parent_scope,
            composite_types_t type,
            compiler::symbol_element* symbol,
            element_type_t element_type = element_type_t::composite_type);

        field_map_t& fields();

        type_map_t& type_parameters();

        composite_types_t type() const;

    protected:
        bool on_initialize(
            common::result& r,
            compiler::program* program) override;

    private:
        field_map_t _fields {};
        composite_types_t _type;
        type_map_t _type_parameters {};
    };

};

