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

#pragma once

#include <map>
#include <string>
#include <vector>
#include "terp.h"
#include "label.h"

namespace basecode::vm {

    enum class instruction_block_type_t {
        implicit,
        procedure
    };

    class instruction_block {
    public:
        explicit instruction_block(instruction_block_type_t type);

        virtual ~instruction_block();

        void rts();

        void dup();

        void nop();

        void exit();

        void clear_labels();

        void clear_blocks();

        void move_f32_to_freg(
            f_registers_t dest_reg,
            float immediate);

        void move_f64_to_freg(
            f_registers_t dest_reg,
            double immediate);

        void move_u8_to_ireg(
            i_registers_t dest_reg,
            uint8_t immediate);

        void move_u16_to_ireg(
            i_registers_t dest_reg,
            uint16_t immediate);

        void move_u32_to_ireg(
            i_registers_t dest_reg,
            uint32_t immediate);

        void move_u64_to_ireg(
            i_registers_t dest_reg,
            uint64_t immediate);

        void move_ireg_to_ireg(
            i_registers_t dest_reg,
            i_registers_t src_reg);

        void swi(uint8_t index);

        void trap(uint8_t index);

        void clear_instructions();

        void push_f32(float value);

        void push_f64(double value);

        void push_u8(uint8_t value);

        void push_u16(uint16_t value);

        void push_u32(uint32_t value);

        void push_u64(uint64_t value);

        i_registers_t allocate_ireg();

        f_registers_t allocate_freg();

        void pop_u8(i_registers_t reg);

        void pop_f32(f_registers_t reg);

        void pop_f64(f_registers_t reg);

        void pop_u16(i_registers_t reg);

        void pop_u32(i_registers_t reg);

        void pop_u64(i_registers_t reg);

        bool reserve_ireg(i_registers_t reg);

        bool reserve_freg(f_registers_t reg);

        void jump_indirect(i_registers_t reg);

        instruction_block_type_t type() const;

        void call(const std::string& proc_name);

        void add_block(instruction_block* block);

        void remove_block(instruction_block* block);

        vm::label* make_label(const std::string& name);

        void jump_direct(const std::string& label_name);

    private:
        instruction_block_type_t _type;
        std::vector<instruction_block*> _blocks {};
        std::vector<instruction_t> _instructions {};
        std::map<std::string, vm::label*> _labels {};
        std::set<f_registers_t> _used_float_registers {};
        std::set<i_registers_t> _used_integer_registers {};
        std::map<std::string, size_t> _label_to_instruction_map {};
    };

};

