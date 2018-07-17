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

#include "program.h"
#include "any_type.h"
#include "identifier.h"

namespace basecode::compiler {

    any_type::any_type(
        compiler::block* parent_scope,
        compiler::block* scope) : compiler::composite_type(
                                    parent_scope,
                                    composite_types_t::struct_type,
                                    scope,
                                    nullptr,
                                    element_type_t::any_type) {
    }

    // any_type := struct {
    //      type_info:type;
    //      data:address;
    // };

    bool any_type::on_initialize(
            common::result& r,
            compiler::program* program) {
        symbol(program->make_symbol(parent_scope(), "any"));

        auto block_scope = scope();

        auto type_info_type = program->find_type({ .name = "type" });
        auto address_type = program->find_type({ .name = "address" });

        auto type_info_identifier = program->make_identifier(
            block_scope,
            program->make_symbol(parent_scope(), "type_info"),
            nullptr,
            true);
        type_info_identifier->type(type_info_type);
        auto type_info_field = program->make_field(
            block_scope,
            type_info_identifier);

        auto data_identifier = program->make_identifier(
            block_scope,
            program->make_symbol(parent_scope(), "data"),
            nullptr,
            true);
        data_identifier->type(address_type);
        auto data_field = program->make_field(
            block_scope,
            data_identifier);

        auto& field_map = fields();
        field_map.add(type_info_field);
        field_map.add(data_field);

        return composite_type::on_initialize(r, program);
    }

    compiler::type* any_type::underlying_type() {
        return _underlying_type;
    }

    void any_type::underlying_type(compiler::type* value) {
        _underlying_type = value;
    }

};