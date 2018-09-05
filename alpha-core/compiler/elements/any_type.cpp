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
#include <compiler/scope_manager.h>
#include "program.h"
#include "any_type.h"
#include "identifier.h"
#include "pointer_type.h"
#include "symbol_element.h"

namespace basecode::compiler {

    any_type::any_type(
        compiler::module* module,
        compiler::block* parent_scope,
        compiler::block* scope) : compiler::composite_type(
                                    module,
                                    parent_scope,
                                    composite_types_t::struct_type,
                                    scope,
                                    nullptr,
                                    element_type_t::any_type) {
    }

    type_access_model_t any_type::on_access_model() const {
        return type_access_model_t::pointer;
    }

    bool any_type::on_initialize(compiler::session& session) {
        auto& builder = session.builder();

        symbol(builder.make_symbol(parent_scope(), "any"));

        auto block_scope = scope();

        auto type_info_type = session.scope_manager().find_type({ .name = "type" });
        auto u8_type = session.scope_manager().find_type({ .name = "u8" });
        auto u8_ptr_type = builder.make_pointer_type(
            block_scope,
            qualified_symbol_t { .name = "u8" },
            u8_type);
        auto type_info_ptr_type = builder.make_pointer_type(
            block_scope,
            qualified_symbol_t { .name = "type" },
            type_info_type);

        auto metadata_identifier = builder.make_identifier(
            block_scope,
            builder.make_symbol(parent_scope(), "metadata"),
            nullptr);
        metadata_identifier->type_ref(builder.make_type_reference(
            block_scope,
            qualified_symbol_t {.name = "^type"},
            type_info_ptr_type));
        auto metadata_field = builder.make_field(
            this,
            block_scope,
            metadata_identifier,
            0);

        auto data_identifier = builder.make_identifier(
            block_scope,
            builder.make_symbol(parent_scope(), "data"),
            nullptr);
        data_identifier->type_ref(builder.make_type_reference(
            block_scope,
            qualified_symbol_t {.name = "^u8"},
            u8_ptr_type));
        auto data_field = builder.make_field(
            this,
            block_scope,
            data_identifier,
            metadata_field->end_offset());

        auto& field_map = fields();
        field_map.add(metadata_field);
        field_map.add(data_field);

        return composite_type::on_initialize(session);
    }

};