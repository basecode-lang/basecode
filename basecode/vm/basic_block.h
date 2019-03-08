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

    class basic_block {
    public:
        explicit basic_block(basic_block_type_t type);

        virtual ~basic_block();

    // block support
    public:
        void local(
            local_type_t type,
            const std::string& name,
            int64_t offset = 0,
            const std::string& frame_offset = {});

        void comment(
            const std::string& value,
            uint8_t indent,
            comment_location_t location = comment_location_t::new_line);

        void comment(
            const std::string& value,
            comment_location_t location = comment_location_t::new_line);

        void blank_line();

        void clear_entries();

        common::id_t id() const;

        void label(vm::label* value);

        block_entry_list_t& entries();

        basic_block_type_t type() const;

        ssize_t insertion_point() const;

        void insertion_point(ssize_t value);

        listing_source_file_t* source_file();

        void make_block_entry(const meta_t& meta);

        bool is_current_instruction(op_codes code);

        void make_block_entry(const label_t& label);

        void make_block_entry(const local_t& local);

        bool has_local(const std::string& name) const;

        void make_block_entry(const align_t& section);

        void source_file(listing_source_file_t* value);

        void make_block_entry(const comment_t& comment);

        void make_block_entry(const section_t& section);

        void make_block_entry(const instruction_t& inst);

        void make_block_entry(const data_definition_t& data);

        const vm::local_t* local(const std::string& name) const;

        void frame_offset(const std::string& name, int64_t offset);

    // meta directives
    public:
        void meta_end();

        void meta_begin();

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
            op_sizes inst_size,
            const instruction_operand_t& dest,
            const instruction_operand_t& src,
            const instruction_operand_t& size);

        void fill(
            op_sizes inst_size,
            const instruction_operand_t& dest,
            const instruction_operand_t& value,
            const instruction_operand_t& size);

        // alloc/free
        void alloc(
            op_sizes inst_size,
            const instruction_operand_t& dest,
            const instruction_operand_t& size);

        void free(const instruction_operand_t& addr);

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
            const instruction_operand_t& offset = {});

        // store variations
        void store(
            const instruction_operand_t& dest,
            const instruction_operand_t& src,
            const instruction_operand_t& offset = {});

        // move variations
        void move(
            const instruction_operand_t& dest,
            const instruction_operand_t& src,
            const instruction_operand_t& offset = {});

        void moves(
            const instruction_operand_t& dest,
            const instruction_operand_t& src,
            const instruction_operand_t& offset = {});

        void movez(
            const instruction_operand_t& dest,
            const instruction_operand_t& src,
            const instruction_operand_t& offset = {});

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
            const instruction_operand_t& src,
            const instruction_operand_t& dest);

        void bnz(
            const instruction_operand_t& src,
            const instruction_operand_t& dest);

        // branches
        void bb(const instruction_operand_t& dest);

        void ba(const instruction_operand_t& dest);

        void bg(const instruction_operand_t& dest);

        void bl(const instruction_operand_t& dest);

        void bs(const instruction_operand_t& dest);

        void bo(const instruction_operand_t& dest);

        void bcc(const instruction_operand_t& dest);

        void bcs(const instruction_operand_t& dest);

        void bne(const instruction_operand_t& dest);

        void beq(const instruction_operand_t& dest);

        void bbe(const instruction_operand_t& dest);

        void bae(const instruction_operand_t& dest);

        void bge(const instruction_operand_t& dest);

        void ble(const instruction_operand_t& dest);

        // inc variations
        void inc(const instruction_operand_t& target);

        // dec variations
        void dec(const instruction_operand_t& target);

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

        // push variations
        void push_locals(
            vm::assembler& assembler,
            const std::string& excluded = {});

        void push(const instruction_operand_t& operand);

        void pushm(const instruction_operand_list_t& operands);

        // pop variations
        void pop_locals(
            vm::assembler& assembler,
            const std::string& excluded = {});

        void pop(const instruction_operand_t& dest);

        void popm(const instruction_operand_list_t& operands);

        // calls & jumps
        void call(const instruction_operand_t& target);

        void call_foreign(
            const instruction_operand_t& address,
            const instruction_operand_t& signature_id = {});

        void jump_direct(const instruction_operand_t& target);

        void jump_indirect(const instruction_operand_t& target);

    private:
        void make_set(
            op_codes code,
            op_sizes size,
            const instruction_operand_t& dest);

        void make_branch(
            op_codes code,
            op_sizes size,
            const instruction_operand_t& dest);

        bool apply_operand(
            const instruction_operand_t& operand,
            instruction_t& encoding,
            size_t operand_index);

        void apply_local_range(
            vm::assembler& assembler,
            const local_list_t& locals,
            instruction_operand_list_t& operands,
            bool reverse = false);

        void grouped_named_ranges(
            vm::assembler& assembler,
            const local_list_t& locals,
            const std::string& excluded,
            instruction_operand_list_t& operands,
            bool reverse = false);

        local_list_t sorted_locals() const;

    private:
        common::id_t _id;
        basic_block_type_t _type;
        ssize_t _insertion_point = -1;
        block_entry_list_t _entries {};
        int64_t _recent_inst_index = -1;
        std::map<std::string, size_t> _locals {};
        vm::listing_source_file_t* _source_file = nullptr;
    };

};