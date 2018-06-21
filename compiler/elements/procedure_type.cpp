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

#include "procedure_type.h"
#include "field.h"

namespace basecode::compiler {

    procedure_type::procedure_type(
        element* parent,
        const std::string& name) : compiler::type(parent, element_type_t::proc_type, name) {
    }

    field_map_t& procedure_type::returns() {
        return _returns;
    }

    field_map_t& procedure_type::parameters() {
        return _parameters;
    }

    type_map_t& procedure_type::type_parameters() {
        return _type_parameters;
    }

    procedure_instance_list_t& procedure_type::instances() {
        return _instances;
    }
};