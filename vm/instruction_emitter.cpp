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

#include "terp.h"
#include "instruction_emitter.h"

namespace basecode::vm {

    instruction_emitter::instruction_emitter(
            uint64_t address) : _start_address(address) {
    }

    void instruction_emitter::meta(
            uint32_t line,
            uint16_t column,
            const std::string& file_name,
            const std::string& symbol_name) {
        instruction_t meta_op;
        meta_op.op = op_codes::meta;
        meta_op.size = op_sizes::word;
        meta_op.operands_count = 1;
        meta_op.operands[0].type = operand_encoding_t::flags::integer;
        meta_op.operands[0].value.u64 = 6 + file_name.length() + symbol_name.length();
        _meta_information_list.push_back(meta_information_t{
            .line_number = line,
            .column_number = column,
            .symbol = symbol_name,
            .source_file = file_name,
        });
        _instructions.push_back(meta_op);
    }

    void instruction_emitter::nop() {
        instruction_t no_op;
        no_op.op = op_codes::nop;
        _instructions.push_back(no_op);
    }

    void instruction_emitter::dup() {
        instruction_t dup_op;
        dup_op.op = op_codes::dup;
        _instructions.push_back(dup_op);
    }

    void instruction_emitter::rts() {
        instruction_t rts_op;
        rts_op.op = op_codes::rts;
        _instructions.push_back(rts_op);
    }

    void instruction_emitter::exit() {
        instruction_t exit_op;
        exit_op.op = op_codes::exit;
        _instructions.push_back(exit_op);
    }

    void instruction_emitter::clear() {
        _instructions.clear();
    }

    size_t instruction_emitter::size() const {
        size_t size = 0;
        for (const auto& inst : _instructions)
            size += inst.encoding_size();
        return size;
    }

    size_t instruction_emitter::index() const {
        return _instructions.size() - 1;
    }

    void instruction_emitter::jump_pc_relative(
            op_sizes size,
            operand_encoding_t::flags offset_type,
            uint64_t offset) {
        instruction_t jmp_op;
        jmp_op.op = op_codes::jmp;
        jmp_op.size = size;
        jmp_op.operands_count = 2;
        jmp_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        jmp_op.operands[0].value.r8 = i_registers_t::pc;
        jmp_op.operands[1].type =
            offset_type
            | operand_encoding_t::flags::integer;
        jmp_op.operands[1].value.u64 = offset;
        _instructions.push_back(jmp_op);
    }

    void instruction_emitter::swap_int_register(
            op_sizes size,
            i_registers_t target_index,
            i_registers_t source_index) {
        instruction_t swap_op;
        swap_op.op = op_codes::swap;
        swap_op.size = size;
        swap_op.operands_count = 2;
        swap_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        swap_op.operands[0].value.r8 = target_index;
        swap_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        swap_op.operands[1].value.r8 = source_index;
        _instructions.push_back(swap_op);
    }

    void instruction_emitter::swi(uint8_t index) {
        instruction_t swi_op;
        swi_op.op = op_codes::swi;
        swi_op.size = op_sizes::byte;
        swi_op.operands_count = 1;
        swi_op.operands[0].type = operand_encoding_t::flags::integer;
        swi_op.operands[0].value.u64 = index;
        _instructions.push_back(swi_op);
    }

    void instruction_emitter::trap(uint8_t index) {
        instruction_t trap_op;
        trap_op.op = op_codes::trap;
        trap_op.size = op_sizes::byte;
        trap_op.operands_count = 1;
        trap_op.operands[0].type = operand_encoding_t::flags::integer;
        trap_op.operands[0].value.u64 = index;
        _instructions.push_back(trap_op);
    }

    void instruction_emitter::reserve(size_t count) {
        _instructions.reserve(count);
    }

    uint64_t instruction_emitter::end_address() const {
        return _start_address + size();
    }

    uint64_t instruction_emitter::start_address() const {
        return _start_address;
    }

    void instruction_emitter::jump_subroutine_pc_relative(
            op_sizes size,
            operand_encoding_t::flags offset_type,
            uint64_t offset) {
        instruction_t jsr_op;
        jsr_op.op = op_codes::jsr;
        jsr_op.size = size;
        jsr_op.operands_count = 2;
        jsr_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        jsr_op.operands[0].value.r8 = i_registers_t::pc;
        jsr_op.operands[1].type = offset_type | operand_encoding_t::flags::integer;
        jsr_op.operands[1].value.u64 = offset;
        _instructions.push_back(jsr_op);
    }

    void instruction_emitter::load_with_offset_to_register(
            op_sizes size,
            i_registers_t source_index,
            i_registers_t target_index,
            uint64_t offset) {
        instruction_t load_op;
        load_op.op = op_codes::load;
        load_op.size = size;
        load_op.operands_count = 3;
        load_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        load_op.operands[0].value.r8 = target_index;
        load_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        load_op.operands[1].value.r8 = source_index;
        load_op.operands[2].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant;
        load_op.operands[2].value.u64 = offset;
        _instructions.push_back(load_op);
    }

    void instruction_emitter::add_int_register_to_register(
            op_sizes size,
            i_registers_t target_index,
            i_registers_t lhs_index,
            i_registers_t rhs_index) {
        instruction_t add_op;
        add_op.op = op_codes::add;
        add_op.size = size;
        add_op.operands_count = 3;
        add_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        add_op.operands[0].value.r8 = target_index;
        add_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        add_op.operands[1].value.r8 = lhs_index;
        add_op.operands[2].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        add_op.operands[2].value.r8 = rhs_index;
        _instructions.push_back(add_op);
    }

    void instruction_emitter::load_stack_offset_to_register(
            op_sizes size,
            i_registers_t target_index,
            uint64_t offset) {
        instruction_t load_op;
        load_op.op = op_codes::load;
        load_op.size = size;
        load_op.operands_count = 3;
        load_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        load_op.operands[0].value.r8 = target_index;
        load_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        load_op.operands[1].value.r8 = i_registers_t::sp;
        load_op.operands[2].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant;
        load_op.operands[2].value.u64 = offset;
        _instructions.push_back(load_op);
    }

    void instruction_emitter::store_register_to_stack_offset(
            op_sizes size,
            i_registers_t source_index,
            uint64_t offset) {
        instruction_t store_op;
        store_op.op = op_codes::store;
        store_op.size = size;
        store_op.operands_count = 3;
        store_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        store_op.operands[0].value.r8 = source_index;
        store_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        store_op.operands[1].value.r8 = i_registers_t::sp;
        store_op.operands[2].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant;
        store_op.operands[2].value.u64 = offset;
        _instructions.push_back(store_op);
    }

    void instruction_emitter::divide_int_register_to_register(
            op_sizes size,
            i_registers_t target_index,
            i_registers_t lhs_index,
            i_registers_t rhs_index) {
        instruction_t div_op;
        div_op.op = op_codes::div;
        div_op.size = size;
        div_op.operands_count = 3;
        div_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        div_op.operands[0].value.r8 = target_index;
        div_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        div_op.operands[1].value.r8 = lhs_index;
        div_op.operands[2].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        div_op.operands[2].value.r8 = rhs_index;
        _instructions.push_back(div_op);
    }

    bool instruction_emitter::encode(common::result& r, terp& terp) {
        size_t offset = 0;
        for (auto& inst : _instructions) {
            auto inst_size = inst.encode(r, terp.heap(), _start_address + offset);
            if (inst_size == 0)
                return false;
            offset += inst_size;
        }
        return true;
    }

    void instruction_emitter::store_with_offset_from_register(
            op_sizes size,
            i_registers_t source_index,
            i_registers_t target_index,
            uint64_t offset) {
        instruction_t store_op;
        store_op.op = op_codes::store;
        store_op.size = size;
        store_op.operands_count = 3;
        store_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        store_op.operands[0].value.r8 = source_index;
        store_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        store_op.operands[1].value.r8 = target_index;
        store_op.operands[2].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant;
        store_op.operands[2].value.u64 = offset;
        _instructions.push_back(store_op);
    }

    void instruction_emitter::subtract_int_register_to_register(
            op_sizes size,
            i_registers_t target_index,
            i_registers_t lhs_index,
            i_registers_t rhs_index) {
        instruction_t sub_op;
        sub_op.op = op_codes::sub;
        sub_op.size = size;
        sub_op.operands_count = 3;
        sub_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        sub_op.operands[0].value.r8 = target_index;
        sub_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        sub_op.operands[1].value.r8 = lhs_index;
        sub_op.operands[2].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        sub_op.operands[2].value.r8 = rhs_index;
        _instructions.push_back(sub_op);
    }

    void instruction_emitter::subtract_int_constant_from_register(
            op_sizes size,
            i_registers_t target_index,
            i_registers_t lhs_index,
            uint64_t rhs_value) {
        instruction_t sub_op;
        sub_op.op = op_codes::sub;
        sub_op.size = size;
        sub_op.operands_count = 3;
        sub_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        sub_op.operands[0].value.r8 = target_index;
        sub_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        sub_op.operands[1].value.r8 = lhs_index;
        sub_op.operands[2].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant;
        sub_op.operands[2].value.u64 = rhs_value;
        _instructions.push_back(sub_op);
    }

    void instruction_emitter::multiply_int_register_to_register(
            op_sizes size,
            i_registers_t target_index,
            i_registers_t lhs_index,
            i_registers_t rhs_index) {
        instruction_t mul_op;
        mul_op.op = op_codes::mul;
        mul_op.size = size;
        mul_op.operands_count = 3;
        mul_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        mul_op.operands[0].value.r8 = target_index;
        mul_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        mul_op.operands[1].value.r8 = lhs_index;
        mul_op.operands[2].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        mul_op.operands[2].value.r8 = rhs_index;
        _instructions.push_back(mul_op);
    }

    void instruction_emitter::move_int_constant_to_register(
            op_sizes size,
            uint64_t value,
            i_registers_t index) {
        instruction_t move_op;
        move_op.op = op_codes::move;
        move_op.size = size;
        move_op.operands_count = 2;
        move_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant;
        move_op.operands[0].value.u64 = value;
        move_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        move_op.operands[1].value.r8 = index;
        _instructions.push_back(move_op);
    }

    void instruction_emitter::branch_pc_relative_if_equal(
            op_sizes size,
            operand_encoding_t::flags offset_type,
            uint64_t offset) {
        instruction_t branch_op;
        branch_op.op = op_codes::beq;
        branch_op.size = size;
        branch_op.operands_count = 2;
        branch_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        branch_op.operands[0].value.r8 = i_registers_t::pc;
        branch_op.operands[1].type =
            offset_type
            | operand_encoding_t::flags::integer;
        branch_op.operands[1].value.u64 = offset;
        _instructions.push_back(branch_op);
    }

    void instruction_emitter::jump_direct(uint64_t address) {
        instruction_t jmp_op;
        jmp_op.op = op_codes::jmp;
        jmp_op.size = op_sizes::qword;
        jmp_op.operands_count = 1;
        jmp_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant;
        jmp_op.operands[0].value.u64 = address;
        _instructions.push_back(jmp_op);
    }

    void instruction_emitter::branch_pc_relative_if_not_equal(
            op_sizes size,
            operand_encoding_t::flags offset_type,
            uint64_t offset) {
        instruction_t branch_op;
        branch_op.op = op_codes::bne;
        branch_op.size = size;
        branch_op.operands_count = 2;
        branch_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        branch_op.operands[0].value.r8 = i_registers_t::pc;
        branch_op.operands[1].type =
            offset_type
            | operand_encoding_t::flags::integer;
        branch_op.operands[1].value.u64 = offset;
        _instructions.push_back(branch_op);
    }

    void instruction_emitter::compare_int_register_to_register(
            op_sizes size,
            i_registers_t lhs_index,
            i_registers_t rhs_index) {
        instruction_t cmp_op;
        cmp_op.op = op_codes::cmp;
        cmp_op.size = size;
        cmp_op.operands_count = 2;
        cmp_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        cmp_op.operands[0].value.r8 = lhs_index;
        cmp_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        cmp_op.operands[1].value.r8 = rhs_index;
        _instructions.push_back(cmp_op);
    }

    void instruction_emitter::compare_int_register_to_constant(
            op_sizes size,
            i_registers_t index,
            uint64_t value) {
        instruction_t cmp_op;
        cmp_op.op = op_codes::cmp;
        cmp_op.size = size;
        cmp_op.operands_count = 2;
        cmp_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        cmp_op.operands[0].value.r8 = index;
        cmp_op.operands[1].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant;
        cmp_op.operands[1].value.u64 = value;
        _instructions.push_back(cmp_op);
    }

    void instruction_emitter::branch_if_equal(uint64_t address) {
        instruction_t branch_op;
        branch_op.op = op_codes::beq;
        branch_op.size = op_sizes::qword;
        branch_op.operands_count = 1;
        branch_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant;
        branch_op.operands[0].value.u64 = address;
        _instructions.push_back(branch_op);
    }

    void instruction_emitter::push_float_constant(double value) {
        instruction_t push_op;
        push_op.op = op_codes::push;
        push_op.operands_count = 1;
        push_op.operands[0].type = operand_encoding_t::flags::constant;
        push_op.operands[0].value.d64 = value;
        _instructions.push_back(push_op);
    }

    void instruction_emitter::branch_if_not_equal(uint64_t address) {
        instruction_t branch_op;
        branch_op.op = op_codes::bne;
        branch_op.size = op_sizes::qword;
        branch_op.operands_count = 1;
        branch_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant;
        branch_op.operands[0].value.u64 = address;
        _instructions.push_back(branch_op);
    }

    void instruction_emitter::branch_if_lesser(uint64_t address) {
        instruction_t branch_op;
        branch_op.op = op_codes::bl;
        branch_op.size = op_sizes::qword;
        branch_op.operands_count = 1;
        branch_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant;
        branch_op.operands[0].value.u64 = address;
        _instructions.push_back(branch_op);
    }

    void instruction_emitter::branch_if_greater(uint64_t address) {
        instruction_t branch_op;
        branch_op.op = op_codes::bg;
        branch_op.size = op_sizes::qword;
        branch_op.operands_count = 1;
        branch_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant;
        branch_op.operands[0].value.u64 = address;
        _instructions.push_back(branch_op);
    }

    void instruction_emitter::pop_float_register(i_registers_t index) {
        instruction_t pop_op;
        pop_op.op = op_codes::pop;
        pop_op.operands_count = 1;
        pop_op.operands[0].type = operand_encoding_t::flags::reg;
        pop_op.operands[0].value.r8 = index;
        _instructions.push_back(pop_op);
    }

    void instruction_emitter::dec(op_sizes size, i_registers_t index) {
        instruction_t dec_op;
        dec_op.op = op_codes::dec;
        dec_op.operands_count = 1;
        dec_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        dec_op.operands[0].value.r8 = index;
        _instructions.push_back(dec_op);
    }

    void instruction_emitter::inc(op_sizes size, i_registers_t index) {
        instruction_t inc_op;
        inc_op.op = op_codes::inc;
        inc_op.operands_count = 1;
        inc_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        inc_op.operands[0].value.r8 = index;
        _instructions.push_back(inc_op);
    }

    void instruction_emitter::jump_subroutine_direct(uint64_t address) {
        instruction_t jsr_op;
        jsr_op.op = op_codes::jsr;
        jsr_op.size = op_sizes::qword;
        jsr_op.operands_count = 1;
        jsr_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant;
        jsr_op.operands[0].value.u64 = address;
        _instructions.push_back(jsr_op);
    }

    void instruction_emitter::branch_if_lesser_or_equal(uint64_t address) {
        instruction_t branch_op;
        branch_op.op = op_codes::ble;
        branch_op.size = op_sizes::qword;
        branch_op.operands_count = 1;
        branch_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant;
        branch_op.operands[0].value.u64 = address;
        _instructions.push_back(branch_op);
    }

    void instruction_emitter::branch_if_greater_or_equal(uint64_t address) {
        instruction_t branch_op;
        branch_op.op = op_codes::bge;
        branch_op.size = op_sizes::qword;
        branch_op.operands_count = 1;
        branch_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::constant;
        branch_op.operands[0].value.u64 = address;
        _instructions.push_back(branch_op);
    }

    void instruction_emitter::jump_subroutine_indirect(i_registers_t index) {
        instruction_t jsr_op;
        jsr_op.op = op_codes::jsr;
        jsr_op.size = op_sizes::qword;
        jsr_op.operands_count = 1;
        jsr_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        jsr_op.operands[0].value.r8 = index;
        _instructions.push_back(jsr_op);
    }

    void instruction_emitter::push_int_constant(op_sizes size, uint64_t value) {
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

    void instruction_emitter::pop_int_register(op_sizes size, i_registers_t index) {
        instruction_t pop_op;
        pop_op.op = op_codes::pop;
        pop_op.size = size;
        pop_op.operands_count = 1;
        pop_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        pop_op.operands[0].value.r8 = index;
        _instructions.push_back(pop_op);
    }

    void instruction_emitter::push_int_register(op_sizes size, i_registers_t index) {
        instruction_t push_op;
        push_op.op = op_codes::push;
        push_op.size = size;
        push_op.operands_count = 1;
        push_op.operands[0].type =
            operand_encoding_t::flags::integer
            | operand_encoding_t::flags::reg;
        push_op.operands[0].value.r8 = index;
        _instructions.push_back(push_op);
    }

};