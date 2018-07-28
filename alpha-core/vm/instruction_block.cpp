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

#include "instruction_block.h"
#include "terp.h"

namespace basecode::vm {

    instruction_block::instruction_block(
            instruction_block* parent,
            instruction_block_type_t type): _stack_frame(parent),
                                            _type(type),
                                            _parent(parent) {
    }

    instruction_block::~instruction_block() {
        clear_blocks();
        clear_entries();

        for (const auto& it : _labels)
            delete it.second;
        _labels.clear();
    }

    void instruction_block::rts() {
        instruction_t rts_op;
        rts_op.op = op_codes::rts;
        make_block_entry(rts_op);
    }

    void instruction_block::dup() {
        instruction_t dup_op;
        dup_op.op = op_codes::dup;
        make_block_entry(dup_op);
    }

    void instruction_block::nop() {
        instruction_t no_op;
        no_op.op = op_codes::nop;
        make_block_entry(no_op);
    }

    void instruction_block::exit() {
        instruction_t exit_op;
        exit_op.op = op_codes::exit;
        make_block_entry(exit_op);
    }

    void instruction_block::memo() {
        _entries.push_back(block_entry_t());
    }

    void instruction_block::disassemble() {
        disassemble(this);
    }

    void instruction_block::clear_blocks() {
        _blocks.clear();
    }

    block_entry_t* instruction_block::current_entry() {
        if (_entries.empty())
            return nullptr;
        return &_entries.back();
    }

    // sections
    void instruction_block::section(section_t type) {
        make_block_entry(type);
    }

    // data definitions
    void instruction_block::align(uint8_t size) {
        make_block_entry(align_t {
            .size = size
        });
    }

    void instruction_block::byte(uint8_t value) {
        make_block_entry(data_definition_t {
            .size = op_sizes::byte,
            .value = value,
            .type = data_definition_type_t::initialized,
        });
    }

    void instruction_block::word(uint16_t value) {
        make_block_entry(data_definition_t {
            .size = op_sizes::word,
            .value = value,
            .type = data_definition_type_t::initialized,
        });
    }

    void instruction_block::dword(uint32_t value) {
        make_block_entry(data_definition_t {
            .size = op_sizes::dword,
            .value = value,
            .type = data_definition_type_t::initialized,
        });
    }

    void instruction_block::qword(uint64_t value) {
        make_block_entry(data_definition_t {
            .size = op_sizes::qword,
            .value = value,
            .type = data_definition_type_t::initialized,
        });
    }

    void instruction_block::reserve_byte(size_t count) {
        make_block_entry(data_definition_t {
            .size = op_sizes::byte,
            .value = count,
            .type = data_definition_type_t::uninitialized,
        });
    }

    void instruction_block::reserve_word(size_t count) {
        make_block_entry(data_definition_t {
            .size = op_sizes::word,
            .value = count,
            .type = data_definition_type_t::uninitialized,
        });
    }

    void instruction_block::reserve_dword(size_t count) {
        make_block_entry(data_definition_t {
            .size = op_sizes::dword,
            .value = count,
            .type = data_definition_type_t::uninitialized,
        });
    }

    void instruction_block::reserve_qword(size_t count) {
        make_block_entry(data_definition_t {
            .size = op_sizes::qword,
            .value = count,
            .type = data_definition_type_t::uninitialized,
        });
    }

    void instruction_block::string(const std::string& value) {
        dword(static_cast<uint32_t>(value.length()));
        for (const auto& c : value)
            byte(static_cast<uint8_t>(c));
        byte(0);
    }

    // load variations
    void instruction_block::load_to_ireg_u8(
            i_registers_t dest_reg,
            i_registers_t address_reg,
            int64_t offset) {
        make_load_instruction(op_sizes::byte, dest_reg, address_reg, offset);
    }

    void instruction_block::load_to_ireg_u16(
            i_registers_t dest_reg,
            i_registers_t address_reg,
            int64_t offset) {
        make_load_instruction(op_sizes::word, dest_reg, address_reg, offset);
    }

    void instruction_block::load_to_ireg_u32(
            i_registers_t dest_reg,
            i_registers_t address_reg,
            int64_t offset) {
        make_load_instruction(op_sizes::dword, dest_reg, address_reg, offset);
    }

    void instruction_block::load_to_ireg_u64(
            i_registers_t dest_reg,
            i_registers_t address_reg,
            int64_t offset) {
        make_load_instruction(op_sizes::qword, dest_reg, address_reg, offset);
    }

    // store variations
    void instruction_block::store_from_ireg_u8(
            i_registers_t address_reg,
            i_registers_t src_reg,
            int64_t offset) {
        make_store_instruction(op_sizes::byte, address_reg, src_reg, offset);
    }

    void instruction_block::store_from_ireg_u16(
            i_registers_t address_reg,
            i_registers_t src_reg,
            int64_t offset) {
        make_store_instruction(op_sizes::word, address_reg, src_reg, offset);
    }

    void instruction_block::store_from_ireg_u32(
            i_registers_t address_reg,
            i_registers_t src_reg,
            int64_t offset) {
        make_store_instruction(op_sizes::dword, address_reg, src_reg, offset);
    }

    void instruction_block::store_from_ireg_u64(
            i_registers_t address_reg,
            i_registers_t src_reg,
            int64_t offset) {
        make_store_instruction(op_sizes::qword, address_reg, src_reg, offset);
    }

    // move constant to reg variations
    void instruction_block::move_f32_to_freg(
            f_registers_t dest_reg,
            float immediate) {
        make_move_instruction(op_sizes::dword, dest_reg, immediate);
    }

    void instruction_block::move_f64_to_freg(
            f_registers_t dest_reg,
            double immediate) {
        make_move_instruction(op_sizes::qword, dest_reg, immediate);
    }

    void instruction_block::move_u8_to_ireg(
            i_registers_t dest_reg,
            uint8_t immediate) {
        make_move_instruction(op_sizes::byte, dest_reg, immediate);
    }

    void instruction_block::move_u16_to_ireg(
            i_registers_t dest_reg,
            uint16_t immediate) {
        make_move_instruction(op_sizes::word, dest_reg, immediate);
    }

    void instruction_block::move_u32_to_ireg(
            i_registers_t dest_reg,
            uint32_t immediate) {
        make_move_instruction(op_sizes::dword, dest_reg, immediate);
    }

    void instruction_block::move_u64_to_ireg(
            i_registers_t dest_reg,
            uint64_t immediate) {
        make_move_instruction(op_sizes::qword, dest_reg, immediate);
    }

    void instruction_block::move_ireg_to_ireg(
            i_registers_t dest_reg,
            i_registers_t src_reg) {
        make_move_instruction(op_sizes::qword, dest_reg, src_reg);
    }

    void instruction_block::move_label_to_ireg(
            i_registers_t dest_reg,
            const std::string& label_name) {
        auto label_ref = make_unresolved_label_ref(label_name);

        instruction_t move_op;
        move_op.op = op_codes::move;
        move_op.size = op_sizes::qword;
        move_op.operands_count = 2;
        move_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        move_op.operands[0].value.r8 = dest_reg;
        move_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant
            | operand_encoding_t::flags::unresolved;
        move_op.operands[1].value.u64 = label_ref->id;
        make_block_entry(move_op);
    }

    // not variations
    void instruction_block::not_u8(
            i_registers_t dest_reg,
            i_registers_t src_reg) {
        make_not_instruction(op_sizes::byte, dest_reg, src_reg);
    }

    void instruction_block::not_u16(
            i_registers_t dest_reg,
            i_registers_t src_reg) {
        make_not_instruction(op_sizes::word, dest_reg, src_reg);
    }

    void instruction_block::not_u32(
            i_registers_t dest_reg,
            i_registers_t src_reg) {
        make_not_instruction(op_sizes::dword, dest_reg, src_reg);
    }

    void instruction_block::not_u64(
            i_registers_t dest_reg,
            i_registers_t src_reg) {
        make_not_instruction(op_sizes::qword, dest_reg, src_reg);
    }

    // neg variations
    void instruction_block::neg_u8(
            i_registers_t dest_reg,
            i_registers_t src_reg) {
        make_neg_instruction(op_sizes::byte, dest_reg, src_reg);
    }

    void instruction_block::neg_u16(
            i_registers_t dest_reg,
            i_registers_t src_reg) {
        make_neg_instruction(op_sizes::word, dest_reg, src_reg);
    }

    void instruction_block::neg_u32(
            i_registers_t dest_reg,
            i_registers_t src_reg) {
        make_neg_instruction(op_sizes::dword, dest_reg, src_reg);
    }

    void instruction_block::neg_u64(
            i_registers_t dest_reg,
            i_registers_t src_reg) {
        make_neg_instruction(op_sizes::qword, dest_reg, src_reg);
    }

    // mul variations
    void instruction_block::mul_ireg_by_ireg_u8(
            i_registers_t dest_reg,
            i_registers_t multiplicand_reg,
            i_registers_t multiplier_reg) {
        make_mul_instruction(op_sizes::byte, dest_reg, multiplicand_reg, multiplier_reg);
    }

    void instruction_block::mul_ireg_by_ireg_u16(
            i_registers_t dest_reg,
            i_registers_t multiplicand_reg,
            i_registers_t multiplier_reg) {
        make_mul_instruction(op_sizes::word, dest_reg, multiplicand_reg, multiplier_reg);
    }

    void instruction_block::mul_ireg_by_ireg_u32(
            i_registers_t dest_reg,
            i_registers_t multiplicand_reg,
            i_registers_t multiplier_reg) {
        make_mul_instruction(op_sizes::dword, dest_reg, multiplicand_reg, multiplier_reg);
    }

    void instruction_block::mul_ireg_by_ireg_u64(
            i_registers_t dest_reg,
            i_registers_t multiplicand_reg,
            i_registers_t multiplier_reg) {
        make_mul_instruction(op_sizes::qword, dest_reg, multiplicand_reg, multiplier_reg);
    }

    void instruction_block::make_mul_instruction(
            op_sizes size,
            i_registers_t dest_reg,
            i_registers_t multiplicand_reg,
            i_registers_t multiplier_reg) {
        instruction_t mul_op;
        mul_op.op = op_codes::mul;
        mul_op.size = size;
        mul_op.operands_count = 3;
        mul_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        mul_op.operands[0].value.r8 = dest_reg;
        mul_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        mul_op.operands[1].value.r8 = multiplicand_reg;
        mul_op.operands[2].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        mul_op.operands[2].value.r8 = multiplier_reg;
        make_block_entry(mul_op);
    }

    // add variations
    void instruction_block::add_ireg_by_ireg_u8(
            i_registers_t dest_reg,
            i_registers_t augend_reg,
            i_registers_t addened_reg) {
        make_add_instruction(op_sizes::byte, dest_reg, augend_reg, addened_reg);
    }

    void instruction_block::add_ireg_by_ireg_u16(
            i_registers_t dest_reg,
            i_registers_t augend_reg,
            i_registers_t addened_reg) {
        make_add_instruction(op_sizes::word, dest_reg, augend_reg, addened_reg);
    }

    void instruction_block::add_ireg_by_ireg_u32(
            i_registers_t dest_reg,
            i_registers_t augend_reg,
            i_registers_t addened_reg) {
        make_add_instruction(op_sizes::dword, dest_reg, augend_reg, addened_reg);
    }

    void instruction_block::add_ireg_by_ireg_u64(
            i_registers_t dest_reg,
            i_registers_t augend_reg,
            i_registers_t addened_reg) {
        make_add_instruction(op_sizes::qword, dest_reg, augend_reg, addened_reg);
    }

    // sub variations
    void instruction_block::sub_ireg_by_ireg_u8(
            i_registers_t dest_reg,
            i_registers_t minuend_reg,
            i_registers_t subtrahend_reg) {
        make_sub_instruction(op_sizes::byte, dest_reg, minuend_reg, subtrahend_reg);
    }

    void instruction_block::sub_ireg_by_ireg_u16(
            i_registers_t dest_reg,
            i_registers_t minuend_reg,
            i_registers_t subtrahend_reg) {
        make_sub_instruction(op_sizes::word, dest_reg, minuend_reg, subtrahend_reg);
    }

    void instruction_block::sub_ireg_by_ireg_u32(
            i_registers_t dest_reg,
            i_registers_t minuend_reg,
            i_registers_t subtrahend_reg) {
        make_sub_instruction(op_sizes::dword, dest_reg, minuend_reg, subtrahend_reg);
    }

    void instruction_block::sub_ireg_by_ireg_u64(
            i_registers_t dest_reg,
            i_registers_t minuend_reg,
            i_registers_t subtrahend_reg) {
        make_sub_instruction(op_sizes::qword, dest_reg, minuend_reg, subtrahend_reg);
    }

    void instruction_block::sub_ireg_by_immediate(
            i_registers_t dest_reg,
            i_registers_t minuend_reg,
            uint64_t subtrahend_immediate) {
        make_sub_instruction_immediate(op_sizes::qword, dest_reg, minuend_reg, subtrahend_immediate);
    }

    // div variations
    void instruction_block::div_ireg_by_ireg_u8(
            i_registers_t dest_reg,
            i_registers_t dividend_reg,
            i_registers_t divisor_reg) {
        make_div_instruction(op_sizes::byte, dest_reg, dividend_reg, divisor_reg);
    }

    void instruction_block::div_ireg_by_ireg_u16(
            i_registers_t dest_reg,
            i_registers_t dividend_reg,
            i_registers_t divisor_reg) {
        make_div_instruction(op_sizes::word, dest_reg, dividend_reg, divisor_reg);
    }

    void instruction_block::div_ireg_by_ireg_u32(
            i_registers_t dest_reg,
            i_registers_t dividend_reg,
            i_registers_t divisor_reg) {
        make_div_instruction(op_sizes::dword, dest_reg, dividend_reg, divisor_reg);
    }

    void instruction_block::div_ireg_by_ireg_u64(
            i_registers_t dest_reg,
            i_registers_t dividend_reg,
            i_registers_t divisor_reg) {
        make_div_instruction(op_sizes::qword, dest_reg, dividend_reg, divisor_reg);
    }

    void instruction_block::make_div_instruction(
            op_sizes size,
            i_registers_t dest_reg,
            i_registers_t dividend_reg,
            i_registers_t divisor_reg) {
        instruction_t div_op;
        div_op.op = op_codes::div;
        div_op.size = size;
        div_op.operands_count = 3;
        div_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        div_op.operands[0].value.r8 = dest_reg;
        div_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        div_op.operands[1].value.r8 = dividend_reg;
        div_op.operands[2].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        div_op.operands[2].value.r8 = divisor_reg;
        make_block_entry(div_op);
    }

    // mod variations
    void instruction_block::mod_ireg_by_ireg_u8(
            i_registers_t dest_reg,
            i_registers_t dividend_reg,
            i_registers_t divisor_reg) {
        make_mod_instruction(op_sizes::byte, dest_reg, dividend_reg, divisor_reg);
    }

    void instruction_block::mod_ireg_by_ireg_u16(
            i_registers_t dest_reg,
            i_registers_t dividend_reg,
            i_registers_t divisor_reg) {
        make_mod_instruction(op_sizes::word, dest_reg, dividend_reg, divisor_reg);
    }

    void instruction_block::mod_ireg_by_ireg_u32(
            i_registers_t dest_reg,
            i_registers_t dividend_reg,
            i_registers_t divisor_reg) {
        make_mod_instruction(op_sizes::dword, dest_reg, dividend_reg, divisor_reg);
    }

    void instruction_block::mod_ireg_by_ireg_u64(
            i_registers_t dest_reg,
            i_registers_t dividend_reg,
            i_registers_t divisor_reg) {
        make_mod_instruction(op_sizes::qword, dest_reg, dividend_reg, divisor_reg);
    }

    void instruction_block::make_mod_instruction(
            op_sizes size,
            i_registers_t dest_reg,
            i_registers_t dividend_reg,
            i_registers_t divisor_reg) {
        instruction_t mod_op;
        mod_op.op = op_codes::mod;
        mod_op.size = size;
        mod_op.operands_count = 3;
        mod_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        mod_op.operands[0].value.r8 = dest_reg;
        mod_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        mod_op.operands[1].value.r8 = dividend_reg;
        mod_op.operands[2].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        mod_op.operands[2].value.r8 = divisor_reg;
        make_block_entry(mod_op);
    }

    void instruction_block::swi(uint8_t index) {
        instruction_t swi_op;
        swi_op.op = op_codes::swi;
        swi_op.size = op_sizes::byte;
        swi_op.operands_count = 1;
        swi_op.operands[0].type = operand_encoding_t::flags::integer;
        swi_op.operands[0].value.u64 = index;
        make_block_entry(swi_op);
    }

    void instruction_block::trap(uint8_t index) {
        instruction_t trap_op;
        trap_op.op = op_codes::trap;
        trap_op.size = op_sizes::byte;
        trap_op.operands_count = 1;
        trap_op.operands[0].type = operand_encoding_t::flags::integer;
        trap_op.operands[0].value.u64 = index;
        make_block_entry(trap_op);
    }

    void instruction_block::make_not_instruction(
            op_sizes size,
            i_registers_t dest_reg,
            i_registers_t src_reg) {
        instruction_t not_op;
        not_op.op = op_codes::not_op;
        not_op.size = size;
        not_op.operands_count = 2;
        not_op.operands[0].type = operand_encoding_t::flags::reg | operand_encoding_t::flags::integer;
        not_op.operands[0].value.r8 = dest_reg;
        not_op.operands[1].type = operand_encoding_t::flags::reg | operand_encoding_t::flags::integer;
        not_op.operands[1].value.r8 = src_reg;
        make_block_entry(not_op);
    }

    void instruction_block::make_neg_instruction(
            op_sizes size,
            i_registers_t dest_reg,
            i_registers_t src_reg) {
        instruction_t neg_op;
        neg_op.op = op_codes::neg;
        neg_op.size = size;
        neg_op.operands_count = 2;
        neg_op.operands[0].type = operand_encoding_t::flags::reg | operand_encoding_t::flags::integer;
        neg_op.operands[0].value.r8 = dest_reg;
        neg_op.operands[1].type = operand_encoding_t::flags::reg | operand_encoding_t::flags::integer;
        neg_op.operands[1].value.r8 = src_reg;
        make_block_entry(neg_op);
    }

    void instruction_block::clear_entries() {
        _entries.clear();
    }

    void instruction_block::push_f32(float value) {
        make_float_constant_push_instruction(op_sizes::dword, value);
    }

    void instruction_block::make_move_instruction(
            op_sizes size,
            f_registers_t dest_reg,
            double value) {
        instruction_t move_op;
        move_op.op = op_codes::move;
        move_op.size = size;
        move_op.operands_count = 2;
        move_op.operands[0].type = operand_encoding_t::flags::reg;
        move_op.operands[0].value.r8 = dest_reg;
        move_op.operands[1].type = operand_encoding_t::flags::constant;
        move_op.operands[1].value.d64 = value;
        make_block_entry(move_op);
    }

    void instruction_block::make_move_instruction(
            op_sizes size,
            i_registers_t dest_reg,
            uint64_t value) {
        instruction_t move_op;
        move_op.op = op_codes::move;
        move_op.size = size;
        move_op.operands_count = 2;
        move_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        move_op.operands[0].value.r8 = dest_reg;
        move_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant;
        move_op.operands[1].value.u64 = value;
        make_block_entry(move_op);
    }

    void instruction_block::make_move_instruction(
            op_sizes size,
            i_registers_t dest_reg,
            i_registers_t src_reg) {
        instruction_t move_op;
        move_op.op = op_codes::move;
        move_op.size = size;
        move_op.operands_count = 2;
        move_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        move_op.operands[0].value.r8 = dest_reg;
        move_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        move_op.operands[1].value.r8 = src_reg;
        make_block_entry(move_op);
    }

    void instruction_block::make_swap_instruction(
            op_sizes size,
            i_registers_t dest_reg,
            i_registers_t src_reg) {
        instruction_t swap_op;
        swap_op.op = op_codes::swap;
        swap_op.size = size;
        swap_op.operands_count = 2;
        swap_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        swap_op.operands[0].value.r8 = dest_reg;
        swap_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        swap_op.operands[1].value.r8 = src_reg;
        make_block_entry(swap_op);
    }

    void instruction_block::make_load_instruction(
            op_sizes size,
            i_registers_t dest_reg,
            i_registers_t address_reg,
            int64_t offset) {
        instruction_t load_op;
        load_op.op = op_codes::load;
        load_op.size = size;
        load_op.operands_count = static_cast<uint8_t>(offset != 0 ? 3 : 2);
        load_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        load_op.operands[0].value.r8 = dest_reg;
        load_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        load_op.operands[1].value.r8 = address_reg;
        if (load_op.operands_count == 3) {
            load_op.operands[2].type =
                operand_encoding_t::flags::integer
                | operand_encoding_t::flags::constant;
            if (offset < 0)
                load_op.operands[2].type |= operand_encoding_t::flags::negative;
            load_op.operands[2].value.u64 = static_cast<uint64_t>(offset);
        }
        make_block_entry(load_op);
    }

    void instruction_block::make_store_instruction(
            op_sizes size,
            i_registers_t address_reg,
            i_registers_t src_reg,
            int64_t offset) {
        instruction_t store_op;
        store_op.op = op_codes::store;
        store_op.size = size;
        store_op.operands_count = static_cast<uint8_t>(offset != 0 ? 3 : 2);
        store_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        store_op.operands[0].value.r8 = address_reg;

        store_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        store_op.operands[1].value.r8 = src_reg;

        if (store_op.operands_count == 3) {
            store_op.operands[2].type =
                operand_encoding_t::flags::integer
                | operand_encoding_t::flags::constant;
            if (offset < 0)
                store_op.operands[2].type |= operand_encoding_t::flags::negative;
            store_op.operands[2].value.u64 = static_cast<uint64_t>(offset);
        }
        make_block_entry(store_op);
    }

    instruction_block* instruction_block::parent() {
        return _parent;
    }

    void instruction_block::make_add_instruction(
            op_sizes size,
            i_registers_t dest_reg,
            i_registers_t augend_reg,
            i_registers_t addend_reg) {
        instruction_t add_op;
        add_op.op = op_codes::add;
        add_op.size = size;
        add_op.operands_count = 3;
        add_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        add_op.operands[0].value.r8 = dest_reg;
        add_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        add_op.operands[1].value.r8 = augend_reg;
        add_op.operands[2].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        add_op.operands[2].value.r8 = addend_reg;
        make_block_entry(add_op);
    }

    void instruction_block::make_sub_instruction(
            op_sizes size,
            i_registers_t dest_reg,
            i_registers_t minuend_reg,
            i_registers_t subtrahend_reg) {
        instruction_t sub_op;
        sub_op.op = op_codes::sub;
        sub_op.size = size;
        sub_op.operands_count = 3;
        sub_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        sub_op.operands[0].value.r8 = dest_reg;
        sub_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        sub_op.operands[1].value.r8 = minuend_reg;
        sub_op.operands[2].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        sub_op.operands[2].value.r8 = subtrahend_reg;
        make_block_entry(sub_op);
    }

    void instruction_block::make_sub_instruction_immediate(
            op_sizes size,
            i_registers_t dest_reg,
            i_registers_t minuend_reg,
            uint64_t subtrahend_immediate) {
        instruction_t sub_op;
        sub_op.op = op_codes::sub;
        sub_op.size = size;
        sub_op.operands_count = 3;
        sub_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        sub_op.operands[0].value.r8 = dest_reg;
        sub_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        sub_op.operands[1].value.r8 = minuend_reg;
        sub_op.operands[2].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant;
        sub_op.operands[2].value.u64 = subtrahend_immediate;
        make_block_entry(sub_op);
    }

    // swap variations
    void instruction_block::swap_ireg_with_ireg_u8(
            i_registers_t dest_reg,
            i_registers_t src_reg) {
        make_swap_instruction(op_sizes::byte, dest_reg, src_reg);
    }

    void instruction_block::swap_ireg_with_ireg_u16(
            i_registers_t dest_reg,
            i_registers_t src_reg) {
        make_swap_instruction(op_sizes::word, dest_reg, src_reg);
    }

    void instruction_block::swap_ireg_with_ireg_u32(
            i_registers_t dest_reg,
            i_registers_t src_reg) {
        make_swap_instruction(op_sizes::dword, dest_reg, src_reg);
    }

    void instruction_block::swap_ireg_with_ireg_u64(
            i_registers_t dest_reg,
            i_registers_t src_reg) {
        make_swap_instruction(op_sizes::qword, dest_reg, src_reg);
    }

    stack_frame_t* instruction_block::stack_frame() {
        return &_stack_frame;
    }

    void instruction_block::push_u8(i_registers_t reg) {
        make_push_instruction(op_sizes::byte, reg);
    }

    void instruction_block::push_u16(i_registers_t reg) {
        make_push_instruction(op_sizes::word, reg);
    }

    void instruction_block::push_u32(i_registers_t reg) {
        make_push_instruction(op_sizes::dword, reg);
    }

    void instruction_block::push_f32(f_registers_t reg) {
        make_push_instruction(op_sizes::dword, reg);
    }

    void instruction_block::push_f64(f_registers_t reg) {
        make_push_instruction(op_sizes::qword, reg);
    }

    void instruction_block::push_u64(i_registers_t reg) {
        make_push_instruction(op_sizes::qword, reg);
    }

    void instruction_block::push_f64(double value) {
        make_float_constant_push_instruction(op_sizes::qword, value);
    }

    void instruction_block::push_u8(uint8_t value) {
        make_integer_constant_push_instruction(op_sizes::byte, value);
    }

    void instruction_block::push_u16(uint16_t value) {
        make_integer_constant_push_instruction(op_sizes::word, value);
    }

    void instruction_block::push_u32(uint32_t value) {
        make_integer_constant_push_instruction(op_sizes::dword, value);
    }

    void instruction_block::push_u64(uint64_t value) {
        make_integer_constant_push_instruction(op_sizes::qword, value);
    }

    void instruction_block::pop_u8(i_registers_t reg) {
        make_pop_instruction(op_sizes::byte, reg);
    }

    bool instruction_block::allocate_reg(i_registers_t& reg) {
        return _i_register_allocator.allocate(reg);
    }

    bool instruction_block::allocate_reg(f_registers_t& reg) {
        return _f_register_allocator.allocate(reg);
    }

    // cmp variations
    void instruction_block::cmp_u64(
            i_registers_t lhs_reg,
            i_registers_t rhs_reg) {
        make_cmp_instruction(op_sizes::qword, lhs_reg, rhs_reg);
    }

    void instruction_block::make_cmp_instruction(
            op_sizes size,
            i_registers_t lhs_reg,
            i_registers_t rhs_reg) {
        instruction_t cmp_op;
        cmp_op.op = op_codes::cmp;
        cmp_op.size = size;
        cmp_op.operands_count = 2;
        cmp_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        cmp_op.operands[0].value.r8 = lhs_reg;
        cmp_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        cmp_op.operands[1].value.r8 = rhs_reg;
        make_block_entry(cmp_op);
    }

    // inc variations
    void instruction_block::inc_u8(i_registers_t reg) {
        make_inc_instruction(op_sizes::byte, reg);
    }

    void instruction_block::inc_u16(i_registers_t reg) {
        make_inc_instruction(op_sizes::word, reg);
    }

    void instruction_block::inc_u32(i_registers_t reg) {
        make_inc_instruction(op_sizes::dword, reg);
    }

    void instruction_block::inc_u64(i_registers_t reg) {
        make_inc_instruction(op_sizes::qword, reg);
    }

    // dec variations
    void instruction_block::dec_u8(i_registers_t reg) {
        make_dec_instruction(op_sizes::byte, reg);
    }

    void instruction_block::dec_u16(i_registers_t reg) {
        make_dec_instruction(op_sizes::word, reg);
    }

    void instruction_block::dec_u32(i_registers_t reg) {
        make_dec_instruction(op_sizes::dword, reg);
    }

    void instruction_block::dec_u64(i_registers_t reg) {
        make_dec_instruction(op_sizes::qword, reg);
    }

    // pop variations
    void instruction_block::pop_f32(f_registers_t reg) {
        make_pop_instruction(op_sizes::dword, reg);
    }

    void instruction_block::pop_f64(f_registers_t reg) {
        make_pop_instruction(op_sizes::qword, reg);
    }

    void instruction_block::pop_u16(i_registers_t reg) {
        make_pop_instruction(op_sizes::word, reg);
    }

    void instruction_block::pop_u32(i_registers_t reg) {
        make_pop_instruction(op_sizes::dword, reg);
    }

    void instruction_block::pop_u64(i_registers_t reg) {
        make_pop_instruction(op_sizes::qword, reg);
    }

    void instruction_block::test_mask_branch_if_zero_u8(
            i_registers_t value_reg,
            i_registers_t mask_reg,
            i_registers_t address_reg) {
    }

    void instruction_block::test_mask_branch_if_zero_u16(
            i_registers_t value_reg,
            i_registers_t mask_reg,
            i_registers_t address_reg) {
    }

    void instruction_block::test_mask_branch_if_zero_u32(
            i_registers_t value_reg,
            i_registers_t mask_reg,
            i_registers_t address_reg) {
    }

    void instruction_block::test_mask_branch_if_zero_u64(
            i_registers_t value_reg,
            i_registers_t mask_reg,
            i_registers_t address_reg) {
    }

    void instruction_block::free_reg(i_registers_t reg) {
        _i_register_allocator.free(reg);
    }

    void instruction_block::free_reg(f_registers_t reg) {
        _f_register_allocator.free(reg);
    }

    void instruction_block::setz(i_registers_t dest_reg) {
        instruction_t setz_op;
        setz_op.op = op_codes::setz;
        setz_op.size = op_sizes::qword;
        setz_op.operands_count = 1;
        setz_op.operands[0].type = operand_encoding_t::flags::integer
                                  | operand_encoding_t::flags::reg;
        setz_op.operands[0].value.r8 = dest_reg;
        make_block_entry(setz_op);
    }

    void instruction_block::setnz(i_registers_t dest_reg) {
        instruction_t setnz_op;
        setnz_op.op = op_codes::setnz;
        setnz_op.size = op_sizes::qword;
        setnz_op.operands_count = 1;
        setnz_op.operands[0].type = operand_encoding_t::flags::integer
                                   | operand_encoding_t::flags::reg;
        setnz_op.operands[0].value.r8 = dest_reg;
        make_block_entry(setnz_op);
    }

    void instruction_block::test_mask_branch_if_not_zero_u8(
            i_registers_t value_reg,
            i_registers_t mask_reg,
            i_registers_t address_reg) {
    }

    void instruction_block::test_mask_branch_if_not_zero_u16(
            i_registers_t value_reg,
            i_registers_t mask_reg,
            i_registers_t address_reg) {
    }

    void instruction_block::test_mask_branch_if_not_zero_u32(
            i_registers_t value_reg,
            i_registers_t mask_reg,
            i_registers_t address_reg) {
    }

    void instruction_block::test_mask_branch_if_not_zero_u64(
            i_registers_t value_reg,
            i_registers_t mask_reg,
            i_registers_t address_reg) {
    }

    void instruction_block::jump_indirect(i_registers_t reg) {
        instruction_t jmp_op;
        jmp_op.op = op_codes::jmp;
        jmp_op.size = op_sizes::qword;
        jmp_op.operands_count = 1;
        jmp_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        jmp_op.operands[0].value.r8 = reg;
        make_block_entry(jmp_op);
    }

    instruction_block_type_t instruction_block::type() const {
        return _type;
    }

    void instruction_block::bne(const std::string& label_name) {
        auto label_ref = make_unresolved_label_ref(label_name);

        instruction_t branch_op;
        branch_op.op = op_codes::bne;
        branch_op.size = op_sizes::qword;
        branch_op.operands_count = 1;
        branch_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant
            | operand_encoding_t::flags::unresolved;
        branch_op.operands[0].value.u64 = label_ref->id;
        make_block_entry(branch_op);
    }

    void instruction_block::beq(const std::string& label_name) {
        auto label_ref = make_unresolved_label_ref(label_name);

        instruction_t branch_op;
        branch_op.op = op_codes::beq;
        branch_op.size = op_sizes::qword;
        branch_op.operands_count = 1;
        branch_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant
            | operand_encoding_t::flags::unresolved;
        branch_op.operands[0].value.u64 = label_ref->id;
        make_block_entry(branch_op);
    }

    void instruction_block::call(const std::string& proc_name) {
        auto label_ref = make_unresolved_label_ref(proc_name);

        instruction_t jsr_op;
        jsr_op.op = op_codes::jsr;
        jsr_op.size = op_sizes::qword;
        jsr_op.operands_count = 1;
        jsr_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant
            | operand_encoding_t::flags::unresolved;
        jsr_op.operands[0].value.u64 = label_ref->id;
        make_block_entry(jsr_op);

// XXX: this is a PC-relative encoding
//        instruction_t jsr_op;
//        jsr_op.op = op_codes::jsr;
//        jsr_op.size = size;
//        jsr_op.operands_count = 2;
//        jsr_op.operands[0].type =
//            operand_encoding_t::flags::integer
//            | operand_encoding_t::flags::reg;
//        jsr_op.operands[0].value.r8 = i_registers_t::pc;
//        jsr_op.operands[1].type = offset_type | operand_encoding_t::flags::integer;
//        jsr_op.operands[1].value.u64 = offset;
//        _instructions.push_back(jsr_op);
    }

    target_register_t instruction_block::pop_target_register() {
        if (_target_registers.empty())
            return target_register_t {};
        auto reg = _target_registers.top();
        _target_registers.pop();
        return reg;
    }

    void instruction_block::add_block(instruction_block* block) {
        _blocks.push_back(block);
    }

    void instruction_block::disassemble(instruction_block* block) {
        auto source_file = block->source_file();
        if (source_file == nullptr)
            return;

        size_t index = 0;
        for (auto& entry : block->_entries) {
            source_file->add_blank_lines(entry.blank_lines());

            for (const auto& comment : entry.comments()) {
                source_file->add_source_line(0, fmt::format("; {}", comment));
            }
            for (auto label : entry.labels()) {
                source_file->add_source_line(0, fmt::format("{}:", label->name()));
            }
            switch (entry.type()) {
                case block_entry_type_t::memo: {
                    break;
                }
                case block_entry_type_t::align: {
                    auto align = entry.data<align_t>();
                    source_file->add_source_line(
                        0,
                        fmt::format(".align {}", align->size));
                    break;
                }
                case block_entry_type_t::section: {
                    auto section = entry.data<section_t>();
                    source_file->add_source_line(
                        0,
                        fmt::format(".section '{}'", section_name(*section)));
                    break;
                }
                case block_entry_type_t::instruction: {
                    auto inst = entry.data<instruction_t>();
                    auto stream = inst->disassemble([&](uint64_t id) -> std::string {
                        auto label_ref = block->find_unresolved_label_up(static_cast<common::id_t>(id));
                        return label_ref != nullptr ?
                               label_ref->name :
                               fmt::format("unresolved_ref_id({})", id);
                    });
                    source_file->add_source_line(0, fmt::format("\t{}", stream));
                    break;
                }
                case block_entry_type_t::data_definition: {
                    auto definition = entry.data<data_definition_t>();
                    std::stringstream directive;
                    std::string format_spec;
                    switch (definition->type) {
                        case data_definition_type_t::initialized: {
                            switch (definition->size) {
                                case op_sizes::byte:
                                    directive << ".db";
                                    format_spec = "${:02X}";
                                    break;
                                case op_sizes::word:
                                    directive << ".dw";
                                    format_spec = "${:04X}";
                                    break;
                                case op_sizes::dword:
                                    directive << ".dd";
                                    format_spec = "${:08X}";
                                    break;
                                case op_sizes::qword:
                                    directive << ".dq";
                                    format_spec = "${:016X}";
                                    break;
                                default: {
                                    break;
                                }
                            }
                            break;
                        }
                        case data_definition_type_t::uninitialized: {
                            format_spec = "${:04X}";
                            switch (definition->size) {
                                case op_sizes::byte:
                                    directive << ".rb";
                                    break;
                                case op_sizes::word:
                                    directive << ".rw";
                                    break;
                                case op_sizes::dword:
                                    directive << ".rd";
                                    break;
                                case op_sizes::qword:
                                    directive << ".rq";
                                    break;
                                default: {
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    source_file->add_source_line(
                        0,
                        fmt::format(
                            "\t{:<10} {}",
                            directive.str(),
                            fmt::format(format_spec, definition->value)));
                    break;
                }
            }
            index++;
        }

        for (auto child_block : block->_blocks)
            disassemble(child_block);
    }

    void instruction_block::remove_block(instruction_block* block) {
        auto it = std::find_if(
            _blocks.begin(),
            _blocks.end(),
            [&block](auto each) { return each == block; });
        if (it == _blocks.end())
            return;
        _blocks.erase(it);
    }

    target_register_t* instruction_block::current_target_register() {
        if (_target_registers.empty())
            return nullptr;
        return &_target_registers.top();
    }

    void instruction_block::push_target_register(i_registers_t reg) {
        target_register_t target {
            .type = target_register_type_t::integer,
            .reg = {
                .i = reg
            }
        };
        _target_registers.push(target);
    }

    void instruction_block::push_target_register(f_registers_t reg) {
        target_register_t target {
            .type = target_register_type_t::floating_point,
            .reg = {
                .f = reg
            }
        };
        _target_registers.push(target);
    }

    vm::label* instruction_block::make_label(const std::string& name) {
        auto it = _labels.insert(std::make_pair(name, new vm::label(name)));
        return it.first->second;
    }

    void instruction_block::call_foreign(const std::string& proc_name) {
        auto label_ref = make_unresolved_label_ref(proc_name);

        instruction_t ffi_op;
        ffi_op.op = op_codes::ffi;
        ffi_op.size = op_sizes::qword;
        ffi_op.operands_count = 1;
        ffi_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant
            | operand_encoding_t::flags::unresolved;
        ffi_op.operands[0].value.u64 = label_ref->id;
        make_block_entry(ffi_op);
    }

    void instruction_block::jump_direct(const std::string& label_name) {
        auto label_ref = make_unresolved_label_ref(label_name);

        instruction_t jmp_op;
        jmp_op.op = op_codes::jmp;
        jmp_op.size = op_sizes::qword;
        jmp_op.operands_count = 1;
        jmp_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant
            | operand_encoding_t::flags::unresolved;
        jmp_op.operands[0].value.u64 = label_ref->id;
        make_block_entry(jmp_op);
    }

    listing_source_file_t* instruction_block::source_file() {
        return _source_file;
    }

    label_ref_t* instruction_block::find_unresolved_label_up(common::id_t id) {
        auto current_block = this;
        while (current_block != nullptr) {
            auto it = current_block->_unresolved_labels.find(id);
            if (it != current_block->_unresolved_labels.end()) {
                return &it->second;
            }
            current_block = current_block->_parent;
        }
        return nullptr;
    }

    vm::label* instruction_block::find_label_up(const std::string& label_name) {
        auto current_block = this;
        while (current_block != nullptr) {
            auto it = current_block->_labels.find(label_name);
            if (it != current_block->_labels.end()) {
                return it->second;
            }
            current_block = current_block->_parent;
        }
        return nullptr;
    }

    void instruction_block::make_inc_instruction(op_sizes size, i_registers_t reg) {
        instruction_t inc_op;
        inc_op.op = op_codes::inc;
        inc_op.size = size;
        inc_op.operands_count = 1;
        inc_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        inc_op.operands[0].value.r8 = reg;
        make_block_entry(inc_op);
    }

    void instruction_block::make_dec_instruction(op_sizes size, i_registers_t reg) {
        instruction_t dec_op;
        dec_op.op = op_codes::dec;
        dec_op.size = size;
        dec_op.operands_count = 1;
        dec_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        dec_op.operands[0].value.r8 = reg;
        make_block_entry(dec_op);
    }

    void instruction_block::make_push_instruction(op_sizes size, i_registers_t reg) {
        instruction_t push_op;
        push_op.op = op_codes::push;
        push_op.size = size;
        push_op.operands_count = 1;
        push_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        push_op.operands[0].value.r8 = reg;
        make_block_entry(push_op);
    }

    void instruction_block::make_push_instruction(op_sizes size, f_registers_t reg) {
        instruction_t push_op;
        push_op.op = op_codes::push;
        push_op.size = size;
        push_op.operands_count = 1;
        push_op.operands[0].type = operand_encoding_t::flags::reg;
        push_op.operands[0].value.r8 = reg;
        make_block_entry(push_op);
    }

    void instruction_block::make_pop_instruction(op_sizes size, i_registers_t dest_reg) {
        instruction_t pop_op;
        pop_op.op = op_codes::pop;
        pop_op.size = size;
        pop_op.operands_count = 1;
        pop_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        pop_op.operands[0].value.r8 = dest_reg;
        make_block_entry(pop_op);
    }

    void instruction_block::make_pop_instruction(op_sizes size, f_registers_t dest_reg) {
        instruction_t pop_op;
        pop_op.op = op_codes::pop;
        pop_op.size = size;
        pop_op.operands_count = 1;
        pop_op.operands[0].type = operand_encoding_t::flags::reg;
        pop_op.operands[0].value.r8 = dest_reg;
        make_block_entry(pop_op);
    }

    label_ref_t* instruction_block::make_unresolved_label_ref(const std::string& label_name) {
        auto it = _label_to_unresolved_ids.find(label_name);
        if (it != _label_to_unresolved_ids.end()) {
            auto ref_it = _unresolved_labels.find(it->second);
            if (ref_it != _unresolved_labels.end())
                return &ref_it->second;
        }

        auto label = find_label_up(label_name);
        auto ref_id = common::id_pool::instance()->allocate();
        auto insert_pair = _unresolved_labels.insert(std::make_pair(
            ref_id,
            label_ref_t {
                .id = ref_id,
                .name = label_name,
                .resolved = label
            }));
        _label_to_unresolved_ids.insert(std::make_pair(label_name, ref_id));

        return &insert_pair.first.operator->()->second;
    }

    void instruction_block::make_float_constant_push_instruction(op_sizes size, double value) {
        instruction_t push_op;
        push_op.op = op_codes::push;
        push_op.size = size;
        push_op.operands_count = 1;
        push_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant;
        push_op.operands[0].value.d64 = value;
        make_block_entry(push_op);
    }

    void instruction_block::make_integer_constant_push_instruction(op_sizes size, uint64_t value) {
        instruction_t push_op;
        push_op.op = op_codes::push;
        push_op.size = size;
        push_op.operands_count = 1;
        push_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant;
        push_op.operands[0].value.u64 = value;
        make_block_entry(push_op);
    }

    void instruction_block::make_or_instruction(
            op_sizes size,
            i_registers_t dest_reg,
            i_registers_t value_reg,
            i_registers_t mask_reg) {
        instruction_t or_op;
        or_op.op = op_codes::or_op;
        or_op.size = size;
        or_op.operands_count = 3;
        or_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        or_op.operands[0].value.r8 = dest_reg;
        or_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        or_op.operands[1].value.r8 = value_reg;
        or_op.operands[2].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        or_op.operands[2].value.r8 = mask_reg;
        make_block_entry(or_op);
    }

    void instruction_block::make_xor_instruction(
            op_sizes size,
            i_registers_t dest_reg,
            i_registers_t value_reg,
            i_registers_t mask_reg) {
        instruction_t xor_op;
        xor_op.op = op_codes::xor_op;
        xor_op.size = size;
        xor_op.operands_count = 3;
        xor_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        xor_op.operands[0].value.r8 = dest_reg;
        xor_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        xor_op.operands[1].value.r8 = value_reg;
        xor_op.operands[2].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        xor_op.operands[2].value.r8 = mask_reg;
        make_block_entry(xor_op);
    }

    void instruction_block::make_and_instruction(
            op_sizes size,
            i_registers_t dest_reg,
            i_registers_t value_reg,
            i_registers_t mask_reg) {
        instruction_t and_op;
        and_op.op = op_codes::and_op;
        and_op.size = size;
        and_op.operands_count = 3;
        and_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        and_op.operands[0].value.r8 = dest_reg;
        and_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        and_op.operands[1].value.r8 = value_reg;
        and_op.operands[2].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        and_op.operands[2].value.r8 = mask_reg;
        make_block_entry(and_op);
    }

    void instruction_block::make_shl_instruction(
            op_sizes size,
            i_registers_t dest_reg,
            i_registers_t value_reg,
            i_registers_t amount_reg) {
        instruction_t shift_op;
        shift_op.op = op_codes::shl;
        shift_op.size = size;
        shift_op.operands_count = 3;
        shift_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        shift_op.operands[0].value.r8 = dest_reg;
        shift_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        shift_op.operands[1].value.r8 = value_reg;
        shift_op.operands[2].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        shift_op.operands[2].value.r8 = amount_reg;
        make_block_entry(shift_op);
    }

    void instruction_block::make_rol_instruction(
            op_sizes size,
            i_registers_t dest_reg,
            i_registers_t value_reg,
            i_registers_t amount_reg) {
        instruction_t rotate_op;
        rotate_op.op = op_codes::rol;
        rotate_op.size = size;
        rotate_op.operands_count = 3;
        rotate_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        rotate_op.operands[0].value.r8 = dest_reg;
        rotate_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        rotate_op.operands[1].value.r8 = value_reg;
        rotate_op.operands[2].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        rotate_op.operands[2].value.r8 = amount_reg;
        make_block_entry(rotate_op);
    }

    void instruction_block::make_shr_instruction(
            op_sizes size,
            i_registers_t dest_reg,
            i_registers_t value_reg,
            i_registers_t amount_reg) {
        instruction_t shift_op;
        shift_op.op = op_codes::shr;
        shift_op.size = size;
        shift_op.operands_count = 3;
        shift_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        shift_op.operands[0].value.r8 = dest_reg;
        shift_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        shift_op.operands[1].value.r8 = value_reg;
        shift_op.operands[2].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        shift_op.operands[2].value.r8 = amount_reg;
        make_block_entry(shift_op);
    }

    void instruction_block::make_ror_instruction(
            op_sizes size,
            i_registers_t dest_reg,
            i_registers_t value_reg,
            i_registers_t amount_reg) {
        instruction_t rotate_op;
        rotate_op.op = op_codes::ror;
        rotate_op.size = size;
        rotate_op.operands_count = 3;
        rotate_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        rotate_op.operands[0].value.r8 = dest_reg;
        rotate_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        rotate_op.operands[1].value.r8 = value_reg;
        rotate_op.operands[2].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        rotate_op.operands[2].value.r8 = amount_reg;
        make_block_entry(rotate_op);
    }

    void instruction_block::or_ireg_by_ireg_u64(
            i_registers_t dest_reg,
            i_registers_t lhs_reg,
            i_registers_t rhs_reg) {
        make_or_instruction(op_sizes::qword, dest_reg, lhs_reg, rhs_reg);
    }

    void instruction_block::xor_ireg_by_ireg_u64(
            i_registers_t dest_reg,
            i_registers_t lhs_reg,
            i_registers_t rhs_reg) {
        make_xor_instruction(op_sizes::qword, dest_reg, lhs_reg, rhs_reg);
    }

    void instruction_block::and_ireg_by_ireg_u64(
            i_registers_t dest_reg,
            i_registers_t lhs_reg,
            i_registers_t rhs_reg) {
        make_and_instruction(op_sizes::qword, dest_reg, lhs_reg, rhs_reg);
    }

    void instruction_block::shl_ireg_by_ireg_u64(
            i_registers_t dest_reg,
            i_registers_t lhs_reg,
            i_registers_t rhs_reg) {
        make_shl_instruction(op_sizes::qword, dest_reg, lhs_reg, rhs_reg);
    }

    void instruction_block::shr_ireg_by_ireg_u64(
            i_registers_t dest_reg,
            i_registers_t lhs_reg,
            i_registers_t rhs_reg) {
        make_shr_instruction(op_sizes::qword, dest_reg, lhs_reg, rhs_reg);
    }

    void instruction_block::rol_ireg_by_ireg_u64(
            i_registers_t dest_reg,
            i_registers_t lhs_reg,
            i_registers_t rhs_reg) {
        make_rol_instruction(op_sizes::qword, dest_reg, lhs_reg, rhs_reg);
    }

    void instruction_block::ror_ireg_by_ireg_u64(
            i_registers_t dest_reg,
            i_registers_t lhs_reg,
            i_registers_t rhs_reg) {
        make_ror_instruction(op_sizes::qword, dest_reg, lhs_reg, rhs_reg);
    }

    void instruction_block::make_block_entry(const align_t& align) {
        _entries.push_back(block_entry_t(align));
    }

    void instruction_block::source_file(listing_source_file_t* value) {
        _source_file = value;
    }

    void instruction_block::make_block_entry(const section_t& section) {
        _entries.push_back(block_entry_t(section));
    }

    void instruction_block::make_block_entry(const instruction_t& inst) {
        _entries.push_back(block_entry_t(inst));
    }

    void instruction_block::make_block_entry(const data_definition_t& data) {
        _entries.push_back(block_entry_t(data));
    }

};