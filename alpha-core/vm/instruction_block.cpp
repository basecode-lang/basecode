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

namespace basecode::vm {

    instruction_block::instruction_block(instruction_block_type_t type): _type(type) {
    }

    instruction_block::~instruction_block() {
        clear_blocks();
        clear_labels();
        clear_instructions();
    }

    void instruction_block::rts() {
    }

    void instruction_block::dup() {
    }

    void instruction_block::nop() {
    }

    void instruction_block::exit() {
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

    void instruction_block::move_f32_to_freg(
            f_registers_t dest_reg,
            float immediate) {
    }

    void instruction_block::move_f64_to_freg(
            f_registers_t dest_reg,
            double immediate) {
    }

    void instruction_block::move_u8_to_ireg(
            i_registers_t dest_reg,
            uint8_t immediate) {
    }

    void instruction_block::move_u16_to_ireg(
            i_registers_t dest_reg,
            uint16_t immediate) {
    }

    void instruction_block::move_u32_to_ireg(
            i_registers_t dest_reg,
            uint32_t immediate) {
    }

    void instruction_block::move_u64_to_ireg(
            i_registers_t dest_reg,
            uint64_t immediate) {
    }

    void instruction_block::move_ireg_to_ireg(
            i_registers_t dest_reg,
            i_registers_t src_reg) {
    }

    void instruction_block::swi(uint8_t index) {
    }

    void instruction_block::trap(uint8_t index) {
    }

    void instruction_block::clear_instructions() {
        _instructions.clear();
    }

    void instruction_block::push_f32(float value) {
    }

    void instruction_block::push_f64(double value) {
    }

    void instruction_block::push_u8(uint8_t value) {
    }

    void instruction_block::push_u16(uint16_t value) {
    }

    void instruction_block::push_u32(uint32_t value) {
    }

    void instruction_block::push_u64(uint64_t value) {
    }

    void instruction_block::pop_u8(i_registers_t reg) {
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

    void instruction_block::pop_f32(f_registers_t reg) {
    }

    void instruction_block::pop_f64(f_registers_t reg) {
    }

    void instruction_block::pop_u16(i_registers_t reg) {
    }

    void instruction_block::pop_u32(i_registers_t reg) {
    }

    void instruction_block::pop_u64(i_registers_t reg) {
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
    }

    instruction_block_type_t instruction_block::type() const {
        return _type;
    }

    void instruction_block::call(const std::string& proc_name) {
    }

    void instruction_block::add_block(instruction_block* block) {
        _blocks.push_back(block);
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

    vm::label* instruction_block::make_label(const std::string& name) {
        auto label = new vm::label(name);
        _labels.insert(std::make_pair(name, label));
        _label_to_instruction_map.insert(std::make_pair(name, _instructions.size()));
        return label;
    }

    void instruction_block::jump_direct(const std::string& label_name) {
    }

};