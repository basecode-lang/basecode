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

    class composite_type : public type {
    public:
        explicit composite_type(const std::string& name);

        ~composite_type() override;

        field_map_t& fields() {
            return _fields;
        }

        type_map_t& type_parameters() {
            return _type_parameters;
        }

    private:
        field_map_t _fields {};
        type_map_t _type_parameters {};
    };

};

