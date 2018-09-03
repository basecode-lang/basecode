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
#include "array_type.h"
#include "identifier.h"
#include "pointer_type.h"
#include "symbol_element.h"
#include "type_reference.h"

namespace basecode::compiler {

    std::string array_type::name_for_array(
            compiler::type* entry_type,
            size_t size) {
        return fmt::format(
            "__array_{}_{}__",
            entry_type->symbol()->name(),
            size);
    }

    ///////////////////////////////////////////////////////////////////////////

    array_type::array_type(
            compiler::module* module,
            block* parent_scope,
            compiler::block* scope,
            compiler::type_reference* entry_type,
            size_t size) : compiler::composite_type(
                                module,
                                parent_scope,
                                composite_types_t::struct_type,
                                scope,
                                nullptr,
                                element_type_t::array_type),
                           _size(size),
                           _entry_type(entry_type) {
    }

    bool array_type::on_initialize(compiler::session& session) {
        auto& scope_manager = session.scope_manager();
        auto& builder = session.builder();

        auto type_symbol = builder.make_symbol(
            parent_scope(),
            name_for_array(_entry_type->type(), _size));
        symbol(type_symbol);
        type_symbol->parent_element(this);

        auto block_scope = scope();

        auto u8_type = scope_manager.find_type({.name = "u8"});
        auto u32_type = scope_manager.find_type({.name = "u32"});
        auto ptr_type = builder.make_pointer_type(
            block_scope,
            qualified_symbol_t { .name = "u8" },
            u8_type);
        auto type_info_type = scope_manager.find_type({.name = "type"});

        auto u8_type_ref = builder.make_type_reference(
            block_scope,
            u8_type->symbol()->qualified_symbol(),
            u8_type);
        auto u32_type_ref = builder.make_type_reference(
            block_scope,
            u32_type->symbol()->qualified_symbol(),
            u32_type);
        auto ptr_type_ref = builder.make_type_reference(
            block_scope,
            qualified_symbol_t {.name = "^u8"},
            ptr_type);
        auto type_info_ref = builder.make_type_reference(
            block_scope,
            type_info_type->symbol()->qualified_symbol(),
            type_info_type);

        auto flags_identifier = builder.make_identifier(
            block_scope,
            builder.make_symbol(block_scope, "flags"),
            nullptr);
        flags_identifier->type_ref(u8_type_ref);
        auto flags_field = builder.make_field(
            this,
            block_scope,
            flags_identifier);

        auto length_identifier = builder.make_identifier(
            block_scope,
            builder.make_symbol(block_scope, "length"),
            nullptr);
        length_identifier->type_ref(u32_type_ref);
        auto length_field = builder.make_field(
            this,
            block_scope,
            length_identifier);

        auto capacity_identifier = builder.make_identifier(
            block_scope,
            builder.make_symbol(block_scope, "capacity"),
            nullptr);
        capacity_identifier->type_ref(u32_type_ref);
        auto capacity_field = builder.make_field(
            this,
            block_scope,
            capacity_identifier);

        auto element_type_identifier = builder.make_identifier(
            block_scope,
            builder.make_symbol(block_scope, "element_type"),
            nullptr);
        element_type_identifier->type_ref(type_info_ref);
        auto element_type_field = builder.make_field(
            this,
            block_scope,
            element_type_identifier);

        auto data_identifier = builder.make_identifier(
            block_scope,
            builder.make_symbol(block_scope, "data"),
            nullptr);
        data_identifier->type_ref(ptr_type_ref);
        auto data_field = builder.make_field(
            this,
            block_scope,
            data_identifier);

        auto& field_map = fields();
        field_map.add(flags_field);
        field_map.add(length_field);
        field_map.add(capacity_field);
        field_map.add(element_type_field);
        field_map.add(data_field);

        return composite_type::on_initialize(session);
    }

    uint64_t array_type::size() const {
        return _size;
    }

    void array_type::size(uint64_t value) {
        _size = value;
    }

    compiler::type_reference* array_type::entry_type() {
        return _entry_type;
    }

    type_access_model_t array_type::on_access_model() const {
        return type_access_model_t::pointer;
    }

};