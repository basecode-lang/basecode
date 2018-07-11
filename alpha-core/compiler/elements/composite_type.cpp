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

#include "field.h"
#include "identifier.h"
#include "composite_type.h"

namespace basecode::compiler {

    composite_type::composite_type(
            block* parent_scope,
            composite_types_t type,
            const std::string& name,
            element_type_t element_type) : compiler::type(
                                                parent_scope,
                                                element_type,
                                                name),
                                           _type(type) {
    }

    bool composite_type::on_initialize(
            common::result& r,
            compiler::program* program) {
        size_t size = 0;
        for (auto fld : _fields.as_list())
            size += fld->identifier()->type()->size_in_bytes();
        size_in_bytes(size);
        return true;
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

};