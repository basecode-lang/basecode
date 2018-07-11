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

#pragma once

#include <cstdint>
#include <common/result.h>
#include "terp.h"

namespace basecode::vm {

    class terp;

    class instruction_emitter {
    public:
        instruction_emitter();

        void rts();

        void dup();

        void nop();

        void exit();

        void clear();

        void meta(
            uint32_t line,
            uint16_t column,
            const std::string& file_name,
            const std::string& symbol_name);

        bool encode(
            common::result& r,
            terp& terp,
            uint64_t address);

        size_t size() const;

        size_t index() const;

        void jump_pc_relative(
            op_sizes size,
            operand_encoding_t::flags offset_type,
            uint64_t offset);

        void swap_int_register(
            op_sizes size,
            i_registers_t target_index,
            i_registers_t source_index);

        void swi(uint8_t index);

        void trap(uint8_t index);

        void reserve(size_t count);

        void branch_pc_relative_if_equal(
            op_sizes size,
            operand_encoding_t::flags offset_type,
            uint64_t offset);

        void jump_subroutine_pc_relative(
            op_sizes size,
            operand_encoding_t::flags offset_type,
            uint64_t offset);

        void add_int_register_to_register(
            op_sizes size,
            i_registers_t target_index,
            i_registers_t lhs_index,
            i_registers_t rhs_index);

        void load_with_offset_to_register(
            op_sizes size,
            i_registers_t source_index,
            i_registers_t target_index,
            uint64_t offset);

        void move_int_constant_to_register(
            op_sizes size,
            uint64_t value,
            i_registers_t index);

        void jump_direct(uint64_t address);

        void load_stack_offset_to_register(
            op_sizes size,
            i_registers_t target_index,
            uint64_t offset);

        void store_register_to_stack_offset(
            op_sizes size,
            i_registers_t source_index,
            uint64_t offset);

        void store_with_offset_from_register(
            op_sizes size,
            i_registers_t source_index,
            i_registers_t target_index,
            uint64_t offset);

        void divide_int_register_to_register(
            op_sizes size,
            i_registers_t target_index,
            i_registers_t lhs_index,
            i_registers_t rhs_index);

        void branch_pc_relative_if_not_equal(
            op_sizes size,
            operand_encoding_t::flags offset_type,
            uint64_t offset);

        void compare_int_register_to_constant(
            op_sizes size,
            i_registers_t index,
            uint64_t value);

        void compare_int_register_to_register(
            op_sizes size,
            i_registers_t lhs_index,
            i_registers_t rhs_index);

        void push_float_constant(double value);

        void branch_if_equal(uint64_t address);

        void multiply_int_register_to_register(
            op_sizes size,
            i_registers_t target_index,
            i_registers_t lhs_index,
            i_registers_t rhs_index);

        void subtract_int_register_to_register(
            op_sizes size,
            i_registers_t target_index,
            i_registers_t lhs_index,
            i_registers_t rhs_index);

        void branch_if_lesser(uint64_t address);

        void branch_if_greater(uint64_t address);

        void subtract_int_constant_from_register(
            op_sizes size,
            i_registers_t target_index,
            i_registers_t lhs_index,
            uint64_t rhs_value);

        void branch_if_not_equal(uint64_t address);

        void pop_float_register(i_registers_t index);

        void dec(op_sizes size, i_registers_t index);

        void inc(op_sizes size, i_registers_t index);

        void jump_subroutine_direct(uint64_t address);

        inline instruction_t& operator[](size_t index) {
            return _instructions[index];
        };

        void branch_if_lesser_or_equal(uint64_t address);

        void branch_if_greater_or_equal(uint64_t address);

        void jump_subroutine_indirect(i_registers_t index);

        void push_int_constant(op_sizes size, uint64_t value);

        void pop_int_register(op_sizes size, i_registers_t index);

        void push_int_register(op_sizes size, i_registers_t index);

    private:
        std::vector<instruction_t> _instructions {};
        std::vector<meta_information_t> _meta_information_list {};
    };

};