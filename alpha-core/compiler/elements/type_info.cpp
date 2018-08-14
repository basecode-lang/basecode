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

#include <compiler/session.h>
#include "program.h"
#include "type_info.h"
#include "identifier.h"

namespace basecode::compiler {

    type_info::type_info(
        compiler::block* parent_scope,
        compiler::block* scope) : compiler::composite_type(
                                    parent_scope,
                                    composite_types_t::struct_type,
                                    scope,
                                    nullptr,
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

    bool type_info::on_initialize(compiler::session& session) {
        auto program = &session.program();
        auto& builder = program->builder();
        symbol(builder.make_symbol(parent_scope(), "type"));

        auto block_scope = scope();

        auto string_type = program->find_type({.name = "string"});

        auto name_identifier = builder.make_identifier(
            block_scope,
            builder.make_symbol(parent_scope(), "name"),
            nullptr);
        name_identifier->type(string_type);
        auto name_field = builder.make_field(block_scope, name_identifier);

        fields().add(name_field);

        return composite_type::on_initialize(session);
    }

    type_access_model_t type_info::on_access_model() const {
        return type_access_model_t::pointer;
    }

};