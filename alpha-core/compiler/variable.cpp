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

#include "variable.h"
#include "elements/type.h"

namespace basecode::compiler {

    bool variable_register_t::matches(vm::register_t* other_reg) {
        if (other_reg == nullptr)
            return true;
        return (*other_reg).number == reg.number;
    }

    bool variable_register_t::reserve(compiler::session& session) {
        allocated = session.assembler().allocate_reg(reg);
        return allocated;
    }

    void variable_register_t::release(compiler::session& session) {
        if (!allocated)
            return;
        session.assembler().free_reg(reg);
        allocated = false;
    }

    ///////////////////////////////////////////////////////////////////////////

    bool variable_t::init(compiler::session& session) {
        if (!live)
            return false;

        if (address_loaded)
            return true;

        if (usage == identifier_usage_t::heap) {
            if (!address_reg.reserve(session))
                return false;

            auto& assembler = session.assembler();
            auto block = assembler.current_block();

            block->comment(
                fmt::format(
                    "identifier '{}' address (global)",
                    name),
                4);

            auto label_ref = assembler.make_label_ref(name);
            if (address_offset != 0) {
                block->move_label_to_reg_with_offset(
                    address_reg.reg,
                    label_ref,
                    address_offset);
            } else {
                block->move_label_to_reg(address_reg.reg, label_ref);
            }
        }

        value_reg.reg.type = vm::register_type_t::integer;
        if (type != nullptr) {
            if (type->access_model() == type_access_model_t::value) {
                value_reg.reg.size = vm::op_size_for_byte_size(type->size_in_bytes());
                if (type->number_class() == type_number_class_t::floating_point) {
                    value_reg.reg.type = vm::register_type_t::floating_point;
                }
            } else {
                value_reg.reg.size = vm::op_sizes::qword;
            }
        }

        address_loaded = true;

        return true;
    }

    bool variable_t::read(compiler::session& session) {
        if (!live)
            return false;

        if (!init(session))
            return false;

        std::string type_name = "global";
        if (requires_read) {
            if (!value_reg.reserve(session))
                return false;

            auto& assembler = session.assembler();
            auto block = assembler.current_block();

            block->comment(
                fmt::format(
                    "load identifier '{}' value ({})",
                    name,
                    type_name),
                4);

            if (value_reg.reg.size != vm::op_sizes::qword)
                block->clr(vm::op_sizes::qword, value_reg.reg);

            if (usage == identifier_usage_t::stack) {
                type_name = stack_frame_entry_type_name(frame_entry->type);
                block->load_to_reg(
                    value_reg.reg,
                    vm::register_t::fp(),
                    frame_entry->offset);
            } else {
                block->load_to_reg(value_reg.reg, address_reg.reg);
            }

            requires_read = false;
        }

        return true;
    }

    bool variable_t::write(compiler::session& session) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        auto target_reg = assembler.current_target_register();
        if (target_reg == nullptr)
            return false;

        block->store_from_reg(
            address_reg.reg,
            *target_reg,
            frame_entry != nullptr ? frame_entry->offset : 0);

        written = true;
        requires_read = true;

        return true;
    }

    void variable_t::make_live(compiler::session& session) {
        if (live)
            return;
        live = true;
        address_loaded = false;
        requires_read = address_offset == 0;
    }

    void variable_t::make_dormant(compiler::session& session) {
        if (!live)
            return;
        live = false;
        requires_read = false;
        address_loaded = false;
        value_reg.release(session);
        address_reg.release(session);
    }

};