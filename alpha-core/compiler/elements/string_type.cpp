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
#include "identifier.h"
#include "string_type.h"

namespace basecode::compiler {

    string_type::string_type(
        compiler::block* parent_scope,
        compiler::block* scope) : compiler::composite_type(
                                    parent_scope,
                                    composite_types_t::struct_type,
                                    scope,
                                    nullptr,
                                    element_type_t::string_type) {
    }

    // rodata_str_1:
    //  .db "test string"
    //
    //

    // bss_test_string:
    //  .dw 11      ; length
    //  .dw 256     ; capacity
    //  .dq 0       ; initializer must set

    //
    //  (string struct address)
    //  (constant data address)
    // string_ctor:
    //      LOAD.DW     I0, SP, -16
    //      LOAD.DW     I1, SP, -8
    //      LOAD.DW     I3, I0
    //      LOAD.DW     I2, I0, 4
    //      ALLOC.B     I2, I2
    //      STORE.QW    I2, I0, 8
    //      TBZ         I1, .done
    //      COPY.B      I2, I1, I3
    //  .done:
    //      RTS
    //
    // ; example call
    //  PUSH.QW     rodata_str_1
    //  PUSH.QW     bss_test_string
    //  JSR         string_ctor
    //

    // string_type := struct {
    //      length:u32;
    //      capacity:u32;
    //      data:address;
    // }; 16 bytes

    bool string_type::on_initialize(
            common::result& r,
            compiler::program* program) {
        symbol(program->make_symbol(parent_scope(), "string"));

        auto block_scope = scope();

        auto u32_type = program->find_type({.name = "u32"});
        auto address_type = program->find_type({.name = "address"});

        auto length_identifier = program->make_identifier(
            block_scope,
            program->make_symbol(parent_scope(), "length"),
            nullptr,
            true);
        length_identifier->type(u32_type);
        auto length_field = program->make_field(block_scope, length_identifier);

        auto capacity_identifier = program->make_identifier(
            block_scope,
            program->make_symbol(parent_scope(), "capacity"),
            nullptr,
            true);
        capacity_identifier->type(u32_type);
        auto capacity_field = program->make_field(block_scope, capacity_identifier);

        auto data_identifier = program->make_identifier(
            block_scope,
            program->make_symbol(parent_scope(), "data"),
            nullptr,
            true);
        data_identifier->type(address_type);
        auto data_field = program->make_field(block_scope, data_identifier);

        fields().add(length_field);
        fields().add(capacity_field);
        fields().add(data_field);

        return composite_type::on_initialize(r, program);
    }

};