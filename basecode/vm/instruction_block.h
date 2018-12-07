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

#include <map>
#include <stack>
#include <string>
#include <vector>
#include "terp.h"
#include "vm_types.h"

namespace basecode::vm {

    class instruction_block {
    public:
        explicit instruction_block(instruction_block_type_t type);

        virtual ~instruction_block();

    // block support
    public:
        void blank_line();

        void clear_entries();

        common::id_t id() const;

        bool should_emit() const;

        void should_emit(bool value);

        void label(vm::label* value);

        block_entry_list_t& entries();

        listing_source_file_t* source_file();

        instruction_block_type_t type() const;

        bool is_current_instruction(op_codes code);

        void add_entry(const block_entry_t& entry);

        void make_block_entry(const label_t& label);

        void make_block_entry(const align_t& section);

        void source_file(listing_source_file_t* value);

        void make_block_entry(const comment_t& comment);

        void make_block_entry(const section_t& section);

        void make_block_entry(const instruction_t& inst);

        void make_block_entry(const data_definition_t& data);

        void comment(const std::string& value, uint8_t indent);

    // data definitions
    public:
        void string(
            vm::label* start_label,
            vm::label* data_label,
            const std::string& value);

        void align(uint8_t size);

        void section(section_t type);

        void reserve_byte(size_t count);

        void reserve_word(size_t count);

        void reserve_dword(size_t count);

        void reserve_qword(size_t count);

        void bytes(const std::vector<uint8_t>& values);

        void words(const std::vector<uint16_t>& values);

        void dwords(const std::vector<uint32_t>& values);

        void qwords(const std::vector<data_value_variant_t>& values);

    // instructions
    public:
        void rts();

        void dup();

        void nop();

        void exit();

        // clr
        void clr(
            op_sizes size,
            const instruction_operand_t& dest);

        void clr(const instruction_operand_t& dest);

        // copy & fill
        void copy(
            op_sizes size,
            const register_t& dest_reg,
            const register_t& src_reg,
            uint64_t length);

        void copy(
            op_sizes size,
            const register_t& dest_reg,
            const register_t& src_reg,
            const register_t& size_reg);

        void fill(
            op_sizes size,
            const register_t& dest_reg,
            const register_t& value_reg,
            uint64_t length);

        void fill(
            op_sizes size,
            const register_t& dest_reg,
            const register_t& value_reg,
            const register_t& size_reg);

        void zero(
            op_sizes size,
            const register_t& dest_reg,
            uint64_t length);

        // alloc/free
        void alloc(
            op_sizes size,
            const register_t& dest_reg,
            const register_t& size_reg);

        void free(const register_t& addr_reg);

        // convert
        void convert(
            const instruction_operand_t& dest,
            const instruction_operand_t& src);

        // interrupts and traps
        void swi(uint8_t index);

        void trap(uint8_t index);

        // cmp variations
        void cmp(
            op_sizes size,
            const instruction_operand_t& lhs,
            const instruction_operand_t& rhs);

        void cmp(
            const instruction_operand_t& lhs,
            const instruction_operand_t& rhs);

        // not variations
        void not_op(
            const instruction_operand_t& dest,
            const instruction_operand_t& src);

        // neg variations
        void neg(
            const instruction_operand_t& dest,
            const instruction_operand_t& src);

        // load variations
        void load(
            const instruction_operand_t& dest,
            const instruction_operand_t& src,
            const instruction_operand_t& offset);

        // store variations
        void store(
            const instruction_operand_t& dest,
            const instruction_operand_t& src,
            const instruction_operand_t& offset);

        // move variations
        void move(
            const instruction_operand_t& dest,
            const instruction_operand_t& src,
            const instruction_operand_t& offset);

        void moves(
            const instruction_operand_t& dest,
            const instruction_operand_t& src,
            const instruction_operand_t& offset);

        void movez(
            const instruction_operand_t& dest,
            const instruction_operand_t& src,
            const instruction_operand_t& offset);

        // setxx
        void sets(const instruction_operand_t& dest);

        void setns(const instruction_operand_t& dest);

        void seto(const instruction_operand_t& dest);

        void setno(const instruction_operand_t& dest);

        void seta(const instruction_operand_t& dest);

        void setna(const instruction_operand_t& dest);

        void setae(const instruction_operand_t& dest);

        void setnae(const instruction_operand_t& dest);

        void setb(const instruction_operand_t& dest);

        void setnb(const instruction_operand_t& dest);

        void setbe(const instruction_operand_t& dest);

        void setnbe(const instruction_operand_t& dest);

        void setc(const instruction_operand_t& dest);

        void setnc(const instruction_operand_t& dest);

        void setg(const instruction_operand_t& dest);

        void setng(const instruction_operand_t& dest);

        void setge(const instruction_operand_t& dest);

        void setnge(const instruction_operand_t& dest);

        void setl(const instruction_operand_t& dest);

        void setnl(const instruction_operand_t& dest);

        void setle(const instruction_operand_t& dest);

        void setnle(const instruction_operand_t& dest);

        void setz(const instruction_operand_t& dest);

        void setnz(const instruction_operand_t& dest);

        // bz & bnz
        void bz(
            const register_t& reg,
            const label_ref_t* label_ref);

        void bnz(
            const register_t& reg,
            const label_ref_t* label_ref);

        // branches
        void bb(const label_ref_t* label_ref);

        void ba(const label_ref_t* label_ref);

        void bg(const label_ref_t* label_ref);

        void bl(const label_ref_t* label_ref);

        void bs(const label_ref_t* label_ref);

        void bo(const label_ref_t* label_ref);

        void bcc(const label_ref_t* label_ref);

        void bcs(const label_ref_t* label_ref);

        void bne(const label_ref_t* label_ref);

        void beq(const label_ref_t* label_ref);

        void bbe(const label_ref_t* label_ref);

        void bae(const label_ref_t* label_ref);

        void bge(const label_ref_t* label_ref);

        void ble(const label_ref_t* label_ref);

        // inc variations
        void inc(const register_t& reg);

        // dec variations
        void dec(const register_t& reg);

        // pow variations
        void pow(
            const instruction_operand_t& dest,
            const instruction_operand_t& base,
            const instruction_operand_t& exponent);

        // mul variations
        void mul(
            const instruction_operand_t& dest,
            const instruction_operand_t& multiplicand,
            const instruction_operand_t& multiplier);

        // or variations
        void or_op(
            const instruction_operand_t& dest,
            const instruction_operand_t& lhs,
            const instruction_operand_t& rhs);

        // xor variations
        void xor_op(
            const instruction_operand_t& dest,
            const instruction_operand_t& lhs,
            const instruction_operand_t& rhs);

        // and variations
        void and_op(
            const instruction_operand_t& dest,
            const instruction_operand_t& lhs,
            const instruction_operand_t& rhs);

        // shl variations
        void shl(
            const instruction_operand_t& dest,
            const instruction_operand_t& lhs,
            const instruction_operand_t& rhs);

        // shr variations
        void shr(
            const instruction_operand_t& dest,
            const instruction_operand_t& lhs,
            const instruction_operand_t& rhs);

        // rol variations
        void rol(
            const instruction_operand_t& dest,
            const instruction_operand_t& lhs,
            const instruction_operand_t& rhs);

        // ror variations
        void ror(
            const instruction_operand_t& dest,
            const instruction_operand_t& lhs,
            const instruction_operand_t& rhs);

        // add variations
        void add(
            const instruction_operand_t& dest,
            const instruction_operand_t& augend,
            const instruction_operand_t& addened);

        // sub variations
        void sub(
            const instruction_operand_t& dest,
            const instruction_operand_t& minuend,
            const instruction_operand_t& subtrahend);

        // div variations
        void div(
            const instruction_operand_t& dest,
            const instruction_operand_t& dividend,
            const instruction_operand_t& divisor);

        // mod variations
        void mod(
            const instruction_operand_t& dest,
            const instruction_operand_t& dividend,
            const instruction_operand_t& divisor);

        // swap variations
        void swap_reg_with_reg(
            const register_t& dest_reg,
            const register_t& src_reg);

        // test mask for zero and branch
        void test_mask_branch_if_zero(
            const register_t& value_reg,
            const register_t& mask_reg,
            const register_t& address_reg);

        // test mask for non-zero and branch
        void test_mask_branch_if_not_zero(
            const register_t& value_reg,
            const register_t& mask_reg,
            const register_t& address_reg);

        // push variations
        void push(const instruction_operand_t& operand);

        // pop variations
        void pop(const register_t& reg);

        // calls & jumps
        void call(const label_ref_t* label_ref);

        void call_foreign(uint64_t proc_address);

        void jump_indirect(const register_t& reg);

        void jump_direct(const label_ref_t* label_ref);

    private:
        void make_set(
            op_codes code,
            op_sizes size,
            const instruction_operand_t& dest);

        bool apply_operand(
            const instruction_operand_t& operand,
            instruction_t& encoding,
            size_t operand_index);

        void make_branch(
            op_codes code,
            op_sizes size,
            const label_ref_t* label_ref);

        void make_swap_instruction(
            op_sizes size,
            const register_t& dest_reg,
            const register_t& src_reg);

        void make_pop_instruction(const register_t& dest_reg);

    private:
        common::id_t _id;
        bool _should_emit = true;
        instruction_block_type_t _type;
        block_entry_list_t _entries {};
        int64_t _recent_inst_index = -1;
        vm::listing_source_file_t* _source_file = nullptr;
    };

};

