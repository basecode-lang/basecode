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

namespace basecode::compiler {

    array_type::array_type(
            block* parent_scope,
            compiler::symbol_element* symbol,
            compiler::block* scope,
            compiler::type* entry_type) : compiler::composite_type(
                                                parent_scope,
                                                composite_types_t::struct_type,
                                                scope,
                                                symbol,
                                                element_type_t::array_type),
                                          _entry_type(entry_type) {
    }

    // array_type := struct {
    //      flags:u8;
    //      length:u32;
    //      capacity:u32;
    //      element_type:type;
    //      data:address;
    // };
    bool array_type::on_initialize(
            common::result& r,
            compiler::program* program) {
        auto block_scope = scope();

        auto u8_type = program->find_type({.name = "u8"});
        auto u32_type = program->find_type({.name = "u32"});
        auto type_info_type = program->find_type({.name = "type"});
        auto address_type = program->find_type({.name = "address"});

        auto flags_identifier = program->make_identifier(
            block_scope,
            program->make_symbol(block_scope, "flags"),
            nullptr,
            true);
        flags_identifier->type(u8_type);
        auto flags_field = program->make_field(
            block_scope,
            flags_identifier);

        auto length_identifier = program->make_identifier(
            block_scope,
            program->make_symbol(block_scope, "length"),
            nullptr,
            true);
        length_identifier->type(u32_type);
        auto length_field = program->make_field(
            block_scope,
            length_identifier);

        auto capacity_identifier = program->make_identifier(
            block_scope,
            program->make_symbol(block_scope, "capacity"),
            nullptr,
            true);
        capacity_identifier->type(u32_type);
        auto capacity_field = program->make_field(
            block_scope,
            capacity_identifier);

        auto element_type_identifier = program->make_identifier(
            block_scope,
            program->make_symbol(block_scope, "element_type"),
            nullptr,
            true);
        element_type_identifier->type(type_info_type);
        auto element_type_field = program->make_field(
            block_scope,
            element_type_identifier);

        auto data_identifier = program->make_identifier(
            block_scope,
            program->make_symbol(block_scope, "data"),
            nullptr,
            true);
        data_identifier->type(address_type);
        auto data_field = program->make_field(block_scope, data_identifier);

        auto& field_map = fields();
        field_map.add(flags_field);
        field_map.add(length_field);
        field_map.add(capacity_field);
        field_map.add(element_type_field);
        field_map.add(data_field);

        return composite_type::on_initialize(r, program);
    }

    uint64_t array_type::size() const {
        return _size;
    }

    void array_type::size(uint64_t value) {
        _size = value;
    }

    compiler::type* array_type::entry_type() {
        return _entry_type;
    }

};