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

#include "composite_type.h"
#include "field.h"

namespace basecode::compiler {

    composite_type::composite_type(
            element* parent,
            composite_types_t type,
            const std::string& name) : compiler::type(
                                            parent,
                                            element_type_t::composite_type,
                                            name),
                                       _type(type) {
    }

    field_map_t& composite_type::fields() {
        return _fields;
    }

    type_map_t& composite_type::type_parameters() {
        return _type_parameters;
    }

    composite_types_t composite_type::type() const {
        return _type;
    }

    bool composite_type::on_initialize(common::result& r) {
        return true;
    }

};