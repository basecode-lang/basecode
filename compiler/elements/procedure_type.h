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

#include "type.h"
#include "field.h"

namespace basecode::compiler {

    class procedure_type : public type {
    public:
        procedure_type(
            element* parent,
            const std::string& name);

        field_map_t& returns();

        field_map_t& parameters();

        type_map_t& type_parameters();

    private:
        field_map_t _returns {};
        field_map_t _parameters {};
        type_map_t _type_parameters {};
    };

};

