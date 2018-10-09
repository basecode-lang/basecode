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

        void make_push_instruction(const register_t& reg);

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
            const register_t& dest_reg);

        void clr(const register_t& dest_reg);

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
            const register_t& dest_reg,
            const register_t& src_reg);

        // interrupts and traps
        void swi(uint8_t index);

        void trap(uint8_t index);

        // cmp variations
        void cmp(
            op_sizes size,
            const register_t& lhs_reg,
            uint64_t value);

        void cmp(
            const register_t& lhs_reg,
            const register_t& rhs_reg);

        // not variations
        void not_op(
            const register_t& dest_reg,
            const register_t& src_reg);

        // neg variations
        void neg(
            const register_t& dest_reg,
            const register_t& src_reg);

        // load variations
        void load_to_reg(
            const register_t& dest_reg,
            const register_t& address_reg,
            int64_t offset = 0);

        // store variations
        void store_from_reg(
            const register_t& address_reg,
            const register_t& src_reg,
            int64_t offset = 0);

        // move variations
        void move_sp_to_fp();

        void move_fp_to_sp();

        void move_constant_to_reg(
            const register_t& dest_reg,
            uint64_t immediate);

        void move_constant_to_reg(
            const register_t& dest_reg,
            double immediate);

        void move_reg_to_reg(
            const register_t& dest_reg,
            const register_t& src_reg,
            int64_t offset = 0);

        void moves_reg_to_reg(
            const register_t& dest_reg,
            const register_t& src_reg);

        void movez_reg_to_reg(
            const register_t& dest_reg,
            const register_t& src_reg);

        void move_label_to_reg(
            const register_t& dest_reg,
            const label_ref_t* label_ref,
            int64_t offset = 0);

        // setxx

        void sets(const register_t& dest_reg);

        void setns(const register_t& dest_reg);

        void seto(const register_t& dest_reg);

        void setno(const register_t& dest_reg);

        void seta(const register_t& dest_reg);

        void setna(const register_t& dest_reg);

        void setae(const register_t& dest_reg);

        void setnae(const register_t& dest_reg);

        void setb(const register_t& dest_reg);

        void setnb(const register_t& dest_reg);

        void setbe(const register_t& dest_reg);

        void setnbe(const register_t& dest_reg);

        void setc(const register_t& dest_reg);

        void setnc(const register_t& dest_reg);

        void setg(const register_t& dest_reg);

        void setng(const register_t& dest_reg);

        void setge(const register_t& dest_reg);

        void setnge(const register_t& dest_reg);

        void setl(const register_t& dest_reg);

        void setnl(const register_t& dest_reg);

        void setle(const register_t& dest_reg);

        void setnle(const register_t& dest_reg);

        void setz(const register_t& dest_reg);

        void setnz(const register_t& dest_reg);

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
        void pow_reg_by_reg(
            const register_t& dest_reg,
            const register_t& base_reg,
            const register_t& exponent_reg);

        // mul variations
        void mul_reg_by_reg(
            const register_t& dest_reg,
            const register_t& multiplicand_reg,
            const register_t& multiplier_reg);

        // or variations
        void or_reg_by_reg(
            const register_t& dest_reg,
            const register_t& lhs_reg,
            const register_t& rhs_reg);

        // xor variations
        void xor_reg_by_reg(
            const register_t& dest_reg,
            const register_t& lhs_reg,
            const register_t& rhs_reg);

        // and variations
        void and_reg_by_reg(
            const register_t& dest_reg,
            const register_t& lhs_reg,
            const register_t& rhs_reg);

        // shl variations
        void shl_reg_by_reg(
            const register_t& dest_reg,
            const register_t& lhs_reg,
            const register_t& rhs_reg);

        // shr variations
        void shr_reg_by_reg(
            const register_t& dest_reg,
            const register_t& lhs_reg,
            const register_t& rhs_reg);

        // rol variations
        void rol_reg_by_reg(
            const register_t& dest_reg,
            const register_t& lhs_reg,
            const register_t& rhs_reg);

        // ror variations
        void ror_reg_by_reg(
            const register_t& dest_reg,
            const register_t& lhs_reg,
            const register_t& rhs_reg);

        // add variations
        void add_reg_by_reg(
            const register_t& dest_reg,
            const register_t& augend_reg,
            const register_t& addened_reg);

        // sub variations
        void sub_reg_by_reg(
            const register_t& dest_reg,
            const register_t& minuend_reg,
            const register_t& subtrahend_reg);

        void sub_reg_by_immediate(
            const register_t& dest_reg,
            const register_t& minuend_reg,
            uint64_t subtrahend_immediate);

        // div variations
        void div_reg_by_reg(
            const register_t& dest_reg,
            const register_t& dividend_reg,
            const register_t& divisor_reg);

        // mod variations
        void mod_reg_by_reg(
            const register_t& dest_reg,
            const register_t& dividend_reg,
            const register_t& divisor_reg);

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
        void push_u8(uint8_t value);

        void push_u16(uint16_t value);

        void push_u32(uint32_t value);

        void push_u64(uint64_t value);

        void push(const register_t& reg);

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
            const register_t& dest_reg);

        void make_branch(
            op_codes code,
            op_sizes size,
            const label_ref_t* label_ref);

        void make_clr_instruction(
            op_sizes size,
            const register_t& dest_reg);

        void make_shl_instruction(
            op_sizes size,
            const register_t& dest_reg,
            const register_t& value_reg,
            const register_t& amount_reg);

        void make_rol_instruction(
            op_sizes size,
            const register_t& dest_reg,
            const register_t& value_reg,
            const register_t& amount_reg);

        void make_shr_instruction(
            op_sizes size,
            const register_t& dest_reg,
            const register_t& value_reg,
            const register_t& amount_reg);

        void make_ror_instruction(
            op_sizes size,
            const register_t& dest_reg,
            const register_t& value_reg,
            const register_t& amount_reg);

        void make_and_instruction(
            op_sizes size,
            const register_t& dest_reg,
            const register_t& value_reg,
            const register_t& mask_reg);

        void make_xor_instruction(
            op_sizes size,
            const register_t& dest_reg,
            const register_t& value_reg,
            const register_t& mask_reg);

        void make_or_instruction(
            op_sizes size,
            const register_t& dest_reg,
            const register_t& value_reg,
            const register_t& mask_reg);

        void make_mod_instruction(
            op_sizes size,
            const register_t& dest_reg,
            const register_t& dividend_reg,
            const register_t& divisor_reg);

        void make_div_instruction(
            op_sizes size,
            const register_t& dest_reg,
            const register_t& dividend_reg,
            const register_t& divisor_reg);

        void make_mul_instruction(
            op_sizes size,
            const register_t& dest_reg,
            const register_t& multiplicand_reg,
            const register_t& multiplier_reg);

        void make_cmp_instruction(
            op_sizes size,
            const register_t& lhs_reg,
            const register_t& rhs_reg);

        void make_not_instruction(
            op_sizes size,
            const register_t& dest_reg,
            const register_t& src_reg);

        void make_neg_instruction(
            op_sizes size,
            const register_t& dest_reg,
            const register_t& src_reg);

        void make_inc_instruction(
            op_sizes size,
            const register_t& reg);

        void make_dec_instruction(
            op_sizes size,
            const register_t& reg);

        void make_load_instruction(
            op_sizes size,
            const register_t& dest_reg,
            const register_t& address_reg,
            int64_t offset);

        void make_store_instruction(
            op_sizes size,
            const register_t& address_reg,
            const register_t& src_reg,
            int64_t offset);

        void make_swap_instruction(
            op_sizes size,
            const register_t& dest_reg,
            const register_t& src_reg);

        void make_move_instruction(
            op_sizes size,
            const register_t& dest_reg,
            uint64_t value);

        void make_move_instruction(
            op_sizes size,
            const register_t& dest_reg,
            double value);

        void make_move_instruction(
            op_sizes size,
            op_codes code,
            const register_t& dest_reg,
            const register_t& src_reg,
            int64_t offset = 0);

        void make_add_instruction(
            op_sizes size,
            const register_t& dest_reg,
            const register_t& augend_reg,
            const register_t& addend_reg);

        void make_sub_instruction(
            op_sizes size,
            const register_t& dest_reg,
            const register_t& minuend_reg,
            const register_t& subtrahend_reg);

        void make_sub_instruction_immediate(
            op_sizes size,
            const register_t& dest_reg,
            const register_t& minuend_reg,
            uint64_t subtrahend_immediate);

        void make_integer_constant_push_instruction(
            op_sizes size,
            uint64_t value);

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

