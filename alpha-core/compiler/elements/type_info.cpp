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
#include "type_info.h"

namespace basecode::compiler {

    type_info::type_info(block* parent_scope) : compiler::composite_type(
                                                parent_scope,
                                                composite_types_t::struct_type,
                                                "type",
                                                element_type_t::type_info) {
    }

    // type_category := enum {
    //  numeric,
    //  string,
    //  bool,
    //  array,
    //
    // };
    //
    //
    // type_field := struct {
    //      name:string;
    //      type_info:type;
    // };
    //
    // type_info_type := struct {
    //      name:string;
    //      size_in_bytes:u64;
    //      type:type_category;
    //      fields:[]type_field;
    // };

    bool type_info::on_initialize(
            common::result& r,
            compiler::program* program) {
        auto block_scope = parent_scope();

        auto string_type = program->find_type_down("string");

        auto name_identifier = program->make_identifier(
            block_scope,
            "name",
            nullptr);
        name_identifier->type(string_type);
        auto name_field = program->make_field(block_scope, name_identifier);

        fields().add(name_field);

        return composite_type::on_initialize(r, program);
    }

};