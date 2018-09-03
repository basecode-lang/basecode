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
#include "identifier.h"
#include "string_type.h"
#include "pointer_type.h"
#include "symbol_element.h"

namespace basecode::compiler {

    string_type::string_type(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::block* scope) : compiler::composite_type(
                                        module,
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

    bool string_type::on_initialize(compiler::session& session) {
        auto& builder = session.builder();
        auto& scope_manager = session.scope_manager();

        symbol(builder.make_symbol(parent_scope(), "string"));

        auto block_scope = scope();

        auto u8_type = scope_manager.find_type({.name = "u8"});
        auto u32_type = scope_manager.find_type({.name = "u32"});
        auto ptr_type = builder.make_pointer_type(
            block_scope,
            qualified_symbol_t { .name = "u8" },
            u8_type);

        auto u32_type_ref = builder.make_type_reference(
            block_scope,
            u32_type->symbol()->qualified_symbol(),
            u32_type);
        auto ptr_type_ref = builder.make_type_reference(
            block_scope,
            qualified_symbol_t {.name = "^u8"},
            ptr_type);

        auto length_identifier = builder.make_identifier(
            block_scope,
            builder.make_symbol(parent_scope(), "length"),
            nullptr);
        length_identifier->type_ref(u32_type_ref);
        auto length_field = builder.make_field(
            this,
            block_scope,
            length_identifier);

        auto capacity_identifier = builder.make_identifier(
            block_scope,
            builder.make_symbol(parent_scope(), "capacity"),
            nullptr);
        capacity_identifier->type_ref(u32_type_ref);
        auto capacity_field = builder.make_field(
            this,
            block_scope,
            capacity_identifier);

        auto data_identifier = builder.make_identifier(
            block_scope,
            builder.make_symbol(parent_scope(), "data"),
            nullptr);
        data_identifier->type_ref(ptr_type_ref);
        auto data_field = builder.make_field(
            this,
            block_scope,
            data_identifier);

        fields().add(length_field);
        fields().add(capacity_field);
        fields().add(data_field);

        return composite_type::on_initialize(session);
    }

    type_access_model_t string_type::on_access_model() const {
        return type_access_model_t::pointer;
    }

};