// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#include "instruction_block.h"
#include "terp.h"

namespace basecode::vm {

    instruction_block::instruction_block(
            instruction_block* parent,
            instruction_block_type_t type): _type(type),
                                            _parent(parent) {
    }

    instruction_block::~instruction_block() {
        clear_blocks();
        clear_labels();
        clear_instructions();
    }

    void instruction_block::rts() {
        instruction_t rts_op;
        rts_op.op = op_codes::rts;
        _instructions.push_back(rts_op);
    }

    void instruction_block::dup() {
        instruction_t dup_op;
        dup_op.op = op_codes::dup;
        _instructions.push_back(dup_op);
    }

    void instruction_block::nop() {
        instruction_t no_op;
        no_op.op = op_codes::nop;
        _instructions.push_back(no_op);
    }

    void instruction_block::exit() {
        instruction_t exit_op;
        exit_op.op = op_codes::exit;
        _instructions.push_back(exit_op);
    }

    void instruction_block::disassemble() {
        disassemble(this);
    }

    void instruction_block::clear_labels() {
        for (const auto& it : _labels)
            delete it.second;
        _labels.clear();
        _label_to_instruction_map.clear();
    }

    void instruction_block::clear_blocks() {
        _blocks.clear();
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
        _instructions.push_back(move_op);
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
    }

    void instruction_block::mul_ireg_by_ireg_u16(
            i_registers_t dest_reg,
            i_registers_t multiplicand_reg,
            i_registers_t multiplier_reg) {
    }

    void instruction_block::mul_ireg_by_ireg_u32(
            i_registers_t dest_reg,
            i_registers_t multiplicand_reg,
            i_registers_t multiplier_reg) {
    }

    void instruction_block::mul_ireg_by_ireg_u64(
            i_registers_t dest_reg,
            i_registers_t multiplicand_reg,
            i_registers_t multiplier_reg) {
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

    // div variations
    void instruction_block::div_ireg_by_ireg_u8(
            i_registers_t dest_reg,
            i_registers_t dividend_reg,
            i_registers_t divisor_reg) {
    }

    void instruction_block::div_ireg_by_ireg_u16(
            i_registers_t dest_reg,
            i_registers_t dividend_reg,
            i_registers_t divisor_reg) {
    }

    void instruction_block::div_ireg_by_ireg_u32(
            i_registers_t dest_reg,
            i_registers_t dividend_reg,
            i_registers_t divisor_reg) {
    }

    void instruction_block::div_ireg_by_ireg_u64(
            i_registers_t dest_reg,
            i_registers_t dividend_reg,
            i_registers_t divisor_reg) {
    }

    // mod variations
    void instruction_block::mod_ireg_by_ireg_u8(
            i_registers_t dest_reg,
            i_registers_t dividend_reg,
            i_registers_t divisor_reg) {
    }

    void instruction_block::mod_ireg_by_ireg_u16(
            i_registers_t dest_reg,
            i_registers_t dividend_reg,
            i_registers_t divisor_reg) {
    }

    void instruction_block::mod_ireg_by_ireg_u32(
            i_registers_t dest_reg,
            i_registers_t dividend_reg,
            i_registers_t divisor_reg) {
    }

    void instruction_block::mod_ireg_by_ireg_u64(
            i_registers_t dest_reg,
            i_registers_t dividend_reg,
            i_registers_t divisor_reg) {
    }

    void instruction_block::swi(uint8_t index) {
        instruction_t swi_op;
        swi_op.op = op_codes::swi;
        swi_op.size = op_sizes::byte;
        swi_op.operands_count = 1;
        swi_op.operands[0].type = operand_encoding_t::flags::integer;
        swi_op.operands[0].value.u64 = index;
        _instructions.push_back(swi_op);
    }

    void instruction_block::trap(uint8_t index) {
        instruction_t trap_op;
        trap_op.op = op_codes::trap;
        trap_op.size = op_sizes::byte;
        trap_op.operands_count = 1;
        trap_op.operands[0].type = operand_encoding_t::flags::integer;
        trap_op.operands[0].value.u64 = index;
        _instructions.push_back(trap_op);
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
        _instructions.push_back(not_op);
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
        _instructions.push_back(neg_op);
    }

    void instruction_block::clear_instructions() {
        _instructions.clear();
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
        _instructions.push_back(move_op);
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
        _instructions.push_back(move_op);
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
        _instructions.push_back(move_op);
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
        _instructions.push_back(swap_op);
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
        _instructions.push_back(load_op);
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
        _instructions.push_back(store_op);
    }

    void instruction_block::pop_target_register() {
        if (_target_registers.empty())
            return;
        _target_registers.pop();
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
        _instructions.push_back(add_op);
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
        _instructions.push_back(sub_op);
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

    i_registers_t instruction_block::allocate_ireg() {
        if (!_used_integer_registers.empty()) {
            auto reg = static_cast<i_registers_t>((*_used_integer_registers.rbegin()) + 1);
            _used_integer_registers.insert(reg);
            return reg;
        }
        _used_integer_registers.insert(i_registers_t::i0);
        return *_used_integer_registers.begin();
    }

    f_registers_t instruction_block::allocate_freg() {
        if (!_used_float_registers.empty()) {
            auto reg = static_cast<f_registers_t>((*_used_float_registers.rbegin()) + 1);
            _used_float_registers.insert(reg);
            return reg;
        }
        _used_float_registers.insert(f_registers_t::f0);
        return *_used_float_registers.begin();
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
        _instructions.push_back(cmp_op);
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

    void instruction_block::free_ireg(i_registers_t reg) {
        _used_integer_registers.erase(reg);
    }

    void instruction_block::free_freg(f_registers_t reg) {
        _used_float_registers.erase(reg);
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

    bool instruction_block::reserve_ireg(i_registers_t reg) {
        if (_used_integer_registers.count(reg) > 0)
            return false;
        _used_integer_registers.insert(reg);
        return true;
    }

    bool instruction_block::reserve_freg(f_registers_t reg) {
        if (_used_float_registers.count(reg) > 0)
            return false;
        _used_float_registers.insert(reg);
        return true;
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
        _instructions.push_back(jmp_op);
    }

    instruction_block_type_t instruction_block::type() const {
        return _type;
    }

    void instruction_block::comment(const std::string& value) {
        auto index = _instructions.size();
        auto it = _comments.find(index);
        if (it == _comments.end()) {
            instruction_comments_t comments {};
            comments.lines.push_back(value);
            _comments.insert(std::make_pair(index, comments));
        } else {
            auto& comments = it->second;
            comments.lines.push_back(value);
        }
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
        _instructions.push_back(branch_op);
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
        _instructions.push_back(branch_op);
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
        _instructions.push_back(jsr_op);

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

    void instruction_block::add_block(instruction_block* block) {
        _blocks.push_back(block);
    }

    void instruction_block::disassemble(instruction_block* block) {
        for (const auto& it : block->_labels) {
            fmt::print("{}:\n", it.first);
        }

        size_t index = 0;
        for (const auto& inst : block->_instructions) {
            auto it = block->_comments.find(index);
            if (it != block->_comments.end()) {
                for (const auto& line : it->second.lines) {
                    fmt::print("\t; {}\n", line);
                }
            }
            auto stream = inst.disassemble([&](uint64_t id) -> std::string {
                auto label_ref = block->find_unresolved_label_up(static_cast<id_t>(id));
                return label_ref != nullptr ?
                    label_ref->name :
                    fmt::format("unresolved_ref_id({})", id);
            });
            fmt::print("\t{}\n", stream);
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
        auto label = new vm::label(name);
        _labels.insert(std::make_pair(name, label));
        _label_to_instruction_map.insert(std::make_pair(name, _instructions.size()));
        return label;
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
        _instructions.push_back(ffi_op);
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
        _instructions.push_back(jmp_op);
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
        _instructions.push_back(inc_op);
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
        _instructions.push_back(dec_op);
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
        _instructions.push_back(push_op);
    }

    void instruction_block::make_push_instruction(op_sizes size, f_registers_t reg) {
        instruction_t push_op;
        push_op.op = op_codes::push;
        push_op.size = size;
        push_op.operands_count = 1;
        push_op.operands[0].type = operand_encoding_t::flags::reg;
        push_op.operands[0].value.r8 = reg;
        _instructions.push_back(push_op);
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
        _instructions.push_back(pop_op);
    }

    void instruction_block::make_pop_instruction(op_sizes size, f_registers_t dest_reg) {
        instruction_t pop_op;
        pop_op.op = op_codes::pop;
        pop_op.size = size;
        pop_op.operands_count = 1;
        pop_op.operands[0].type = operand_encoding_t::flags::reg;
        pop_op.operands[0].value.r8 = dest_reg;
        _instructions.push_back(pop_op);
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
        _instructions.push_back(push_op);
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
        _instructions.push_back(push_op);
    }

};