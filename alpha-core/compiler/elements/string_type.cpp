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

#include <common/bytes.h>
#include <compiler/session.h>
#include "program.h"
#include "identifier.h"
#include "initializer.h"
#include "string_type.h"
#include "declaration.h"
#include "pointer_type.h"
#include "symbol_element.h"
#include "string_literal.h"

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

    bool string_type::on_emit_finalizer(
            compiler::session& session,
            compiler::identifier* var) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        block->comment(
            fmt::format("finalize identifier: {}", var->symbol()->name()),
            4);

        auto work_var = session.variable_for_element(var);
        if (work_var == nullptr) {
            session.error(
                var,
                "P051",
                fmt::format("missing assembler variable for {}.", var->label_name()),
                var->location());
            return false;
        }

        work_var->make_live(session);
        defer({
            work_var->make_dormant(session);
        });

        var->emit(session);
        work_var->init(session);

        vm::register_t temp_reg;
        temp_reg.size = vm::op_sizes::qword;
        temp_reg.type = vm::register_type_t::integer;

        if (!assembler.allocate_reg(temp_reg)) {
            session.error(
                var,
                "P052",
                "assembler registers exhausted.",
                var->location());
            return false;
        }

        block->load_to_reg(temp_reg, work_var->address_reg.reg, 8);
        block->free(temp_reg);

        assembler.free_reg(temp_reg);
        return true;
    }

    bool string_type::on_emit_initializer(
            compiler::session& session,
            compiler::identifier* var) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        block->comment(
            fmt::format("initialize identifier: {}", var->symbol()->name()),
            4);

        compiler::string_literal* literal = nullptr;
        auto init = var->initializer();
        if (init != nullptr)
            literal = dynamic_cast<compiler::string_literal*>(init->expression());

        auto work_var = session.variable_for_element(var);
        if (work_var == nullptr) {
            session.error(
                var,
                "P051",
                fmt::format("missing assembler variable for {}.", var->label_name()),
                var->location());
            return false;
        }

        work_var->make_live(session);
        defer({
            work_var->make_dormant(session);
        });

        var->emit(session);
        work_var->init(session);

        vm::register_t temp_reg;
        temp_reg.size = vm::op_sizes::dword;
        temp_reg.type = vm::register_type_t::integer;

        if (!assembler.allocate_reg(temp_reg)) {
            session.error(
                var,
                "P052",
                "assembler registers exhausted.",
                var->location());
            return false;
        }

        auto length = static_cast<uint64_t>(literal != nullptr ? literal->value().length() : 0);
        auto capacity = common::next_power_of_two(std::max<uint64_t>(length, 32));
        block->move_constant_to_reg(temp_reg, length);
        block->store_from_reg(work_var->address_reg.reg, temp_reg);
        block->move_constant_to_reg(temp_reg, capacity);
        block->store_from_reg(work_var->address_reg.reg, temp_reg, 4);

        temp_reg.size = vm::op_sizes::qword;
        block->alloc(vm::op_sizes::byte, temp_reg, temp_reg);
        block->store_from_reg(work_var->address_reg.reg, temp_reg, 8);
        block->zero(vm::op_sizes::byte, temp_reg, capacity);

        if (literal != nullptr) {
            vm::register_t src_reg;
            src_reg.size = vm::op_sizes::dword;
            src_reg.type = vm::register_type_t::integer;

            if (!assembler.allocate_reg(src_reg)) {
                session.error(
                    var,
                    "P052",
                    "assembler registers exhausted.",
                    var->location());
                return false;
            }

            block->comment(
                fmt::format("load string literal address: {}", literal->label_name()),
                4);
            block->move_label_to_reg_with_offset(
                src_reg,
                assembler.make_label_ref(literal->label_name()),
                4);
            block->copy(vm::op_sizes::byte, temp_reg, src_reg, length);
            assembler.free_reg(src_reg);
        }

        assembler.free_reg(temp_reg);
        return true;
    }

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
            builder.make_declaration(block_scope, length_identifier, nullptr),
            0);

        auto capacity_identifier = builder.make_identifier(
            block_scope,
            builder.make_symbol(parent_scope(), "capacity"),
            nullptr);
        capacity_identifier->type_ref(u32_type_ref);
        auto capacity_field = builder.make_field(
            this,
            block_scope,
            builder.make_declaration(block_scope, capacity_identifier, nullptr),
            length_field->end_offset());

        auto data_identifier = builder.make_identifier(
            block_scope,
            builder.make_symbol(parent_scope(), "data"),
            nullptr);
        data_identifier->type_ref(ptr_type_ref);
        auto data_field = builder.make_field(
            this,
            block_scope,
            builder.make_declaration(block_scope, data_identifier, nullptr),
            capacity_field->end_offset());

        auto& field_map = fields();
        field_map.add(length_field);
        field_map.add(capacity_field);
        field_map.add(data_field);

        return composite_type::on_initialize(session);
    }

    type_access_model_t string_type::on_access_model() const {
        return type_access_model_t::pointer;
    }

};