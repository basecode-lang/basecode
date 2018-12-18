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
#include <vm/instruction_block.h>
#include "block.h"
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
            compiler::variable* var) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        block->comment("finalizer", vm::comment_location_t::after_instruction);

        variable_handle_t data_field;
        if (!var->field("data", data_field))
            return false;
        data_field->read();
        block->free(data_field->emit_result().operands.back());

        return true;
    }

    bool string_type::on_emit_initializer(
            compiler::session& session,
            compiler::variable* var) {
        compiler::string_literal* literal = nullptr;

        auto var_ident = dynamic_cast<compiler::identifier*>(var->element());
        auto init = var_ident->initializer();
        if (init == nullptr || init->is_nil())
            return true;

        literal = dynamic_cast<compiler::string_literal*>(init->expression());

        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        block->comment(
            "initializer",
            vm::comment_location_t::after_instruction);

        auto length = static_cast<uint64_t>(literal != nullptr ? literal->value().length() : 0);
        {
            variable_handle_t length_field;
            if (!var->field("length", length_field))
                return false;
            length_field->write(vm::op_sizes::dword, length);
        }

        {
            auto capacity = common::next_power_of_two(std::max<uint64_t>(length, 32));
            variable_handle_t capacity_field;
            if (!var->field("capacity", capacity_field))
                return false;
            capacity_field->write(vm::op_sizes::dword, capacity);

            variable_handle_t data_field;
            if (!var->field("data", data_field))
                return false;
            data_field->read();

            block->alloc(
                vm::op_sizes::byte,
                data_field->emit_result().operands.back(),
                vm::instruction_operand_t(capacity, vm::op_sizes::dword));
            block->fill(
                vm::op_sizes::byte,
                data_field->emit_result().operands.back(),
                vm::instruction_operand_t(static_cast<uint64_t>(0), vm::op_sizes::byte),
                vm::instruction_operand_t(capacity, vm::op_sizes::dword));

            if (literal != nullptr) {
                block->comment(
                    fmt::format("literal: {}", literal->label_name()),
                    vm::comment_location_t::after_instruction);

                variable_handle_t literal_var;
                if (!session.variable(literal, literal_var))
                    return false;
                literal_var->read();

                block->copy(
                    vm::op_sizes::byte,
                    data_field->emit_result().operands.back(),
                    literal_var->emit_result().operands.back(),
                    vm::instruction_operand_t(
                        static_cast<uint64_t>(length),
                        vm::op_sizes::dword));
            }

            data_field->write();
        }

        return true;
    }

    bool string_type::on_initialize(compiler::session& session) {
        auto& builder = session.builder();
        auto& scope_manager = session.scope_manager();

        symbol(builder.make_symbol(parent_scope(), "string"));

        auto block_scope = scope();

        auto u8_type = scope_manager.find_type(qualified_symbol_t("u8"));
        auto u32_type = scope_manager.find_type(qualified_symbol_t("u32"));
        auto ptr_type = builder.make_pointer_type(
            block_scope,
            qualified_symbol_t("u8"),
            u8_type);

        auto u32_type_ref = builder.make_type_reference(
            block_scope,
            u32_type->symbol()->qualified_symbol(),
            u32_type);
        auto ptr_type_ref = builder.make_type_reference(
            block_scope,
            qualified_symbol_t("^u8"),
            ptr_type);

        auto length_identifier = builder.make_identifier(
            block_scope,
            builder.make_symbol(parent_scope(), "length"),
            nullptr);
        length_identifier->type_ref(u32_type_ref);
        block_scope->identifiers().add(length_identifier);
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
        block_scope->identifiers().add(capacity_identifier);
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
        block_scope->identifiers().add(data_identifier);
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