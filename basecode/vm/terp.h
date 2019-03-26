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
#include <set>
#include <string>
#include <memory>
#include <cstdint>
#include <functional>
#include <fmt/format.h>
#include <unordered_set>
#include <unordered_map>
#include <common/result.h>
#include <boost/filesystem.hpp>
#include <boost/unordered_map.hpp>
#include "vm_types.h"

namespace basecode::vm {

    struct icache_entry_t {
        size_t size = 0;
        instruction_t inst {};
        const void* handler = nullptr;
    };

    class instruction_cache {
    public:
        explicit instruction_cache(terp* terp);

        void reset();

        icache_entry_t* fetch(common::result& r);

        icache_entry_t* fetch_at(common::result& r, uint64_t address);

    private:
        terp* _terp = nullptr;
        const register_value_alias_t& _pc;
        boost::unordered_map<uint64_t, icache_entry_t> _cache {};
    };

    ///////////////////////////////////////////////////////////////////////////

    class terp {
    public:
        using trap_callable = std::function<void (terp*)>;

        static constexpr uint8_t  mask_byte_negative  = 0b10000000;
        static constexpr uint16_t mask_word_negative  = 0b1000000000000000;
        static constexpr uint32_t mask_dword_negative = 0b10000000000000000000000000000000;
        static constexpr uint64_t mask_qword_negative = 0b1000000000000000000000000000000000000000000000000000000000000000;

        static constexpr size_t interrupt_vector_table_start = 0;
        static constexpr size_t interrupt_vector_table_size = 16;
        static constexpr size_t interrupt_vector_table_end = interrupt_vector_table_start +
            (sizeof(uint64_t) * interrupt_vector_table_size);

        static constexpr size_t heap_vector_table_start = interrupt_vector_table_end;
        static constexpr size_t heap_vector_table_size = 16;
        static constexpr size_t heap_vector_table_end = heap_vector_table_start
            + (sizeof(uint64_t) * heap_vector_table_size);

        static constexpr size_t program_start = heap_vector_table_end;

        static constexpr uint8_t trap_out_of_memory = 0xff;
        static constexpr uint8_t trap_invalid_ffi_call = 0xfe;
        static constexpr uint8_t trap_invalid_address = 0xfd;

        terp(
            vm::ffi* ffi,
            vm::allocator* allocator,
            size_t heap_size,
            size_t stack_size);

        virtual ~terp();

        void reset();

        uint64_t pop();

        uint64_t peek() const;

        bool has_exited() const;

        inline uint8_t* heap() {
            return _heap;
        }

        size_t heap_size() const;

        void push(uint64_t value);

        size_t stack_size() const;

        bool run(common::result& r);

        void remove_trap(uint8_t index);

        bool initialize(common::result& r);

        void dump_state(uint8_t count = 16);

        std::vector<uint64_t> jump_to_subroutine(
            common::result& r,
            uint64_t address);

        void swi(uint8_t index, uint64_t address);

        const register_file_t& register_file() const;

        void heap_free_space_begin(uint64_t address);

        uint64_t heap_vector(heap_vectors_t vector) const;

        void dump_heap(uint64_t offset, size_t size = 256);

        const meta_information_t& meta_information() const;

        uint64_t read(op_sizes size, uint64_t address) const;

        void heap_vector(heap_vectors_t vector, uint64_t address);

        void write(op_sizes size, uint64_t address, uint64_t value);

        std::string disassemble(common::result& r, uint64_t address);

        void register_trap(uint8_t index, const trap_callable& callable);

    private:
        void set_flags(
            op_sizes size,
            const register_value_alias_t* result,
            const register_value_alias_t* lhs,
            const register_value_alias_t* rhs,
            bool subtract = false,
            const bool* carry_flag_override = nullptr);

        inline bool is_zero(
                op_sizes size,
                const register_value_alias_t& value) {
            switch (size) {
                case op_sizes::byte:
                    return value.b == 0;
                case op_sizes::word:
                    return value.w == 0;
                case op_sizes::dword:
                    return value.dw == 0;
                case op_sizes::qword:
                    return value.qw == 0;
                default:
                    return false;
            }
        }

        inline bool has_carry(
                uint64_t lhs,
                uint64_t rhs,
                op_sizes size) {
            switch (size) {
                case op_sizes::byte:
                    return lhs == UINT8_MAX && rhs > 0;
                case op_sizes::word:
                    return lhs == UINT16_MAX && rhs > 0;
                case op_sizes::dword:
                    return lhs == UINT32_MAX && rhs > 0;
                case op_sizes::qword:
                default:
                    return lhs == UINT64_MAX && rhs > 0;
            }
        }

        inline bool is_negative(
                const register_value_alias_t& value,
                op_sizes size) {
            switch (size) {
                case op_sizes::byte: {
                    return (value.b & mask_byte_negative) != 0;
                }
                case op_sizes::word: {
                    return (value.w & mask_word_negative) != 0;
                }
                case op_sizes::dword: {
                    return (value.dw & mask_dword_negative) != 0;
                }
                case op_sizes::qword:
                default:
                    return (value.qw & mask_qword_negative) != 0;
            }
        }

        inline bool has_overflow(
                const register_value_alias_t& lhs,
                const register_value_alias_t& rhs,
                const register_value_alias_t& result,
                op_sizes size) {
            switch (size) {
                case op_sizes::byte:
                    return ((~(lhs.b ^ rhs.b)) & (lhs.b ^ result.b) & mask_byte_negative) != 0;
                case op_sizes::word:
                    return ((~(lhs.w ^ rhs.w)) & (lhs.w ^ result.w) & mask_word_negative) != 0;
                case op_sizes::dword:
                    return ((~(lhs.dw ^ rhs.dw)) & (lhs.dw ^ result.dw) & mask_dword_negative) != 0;
                case op_sizes::qword:
                default: {
                    return ((~(lhs.qw ^ rhs.qw)) & (lhs.qw ^ result.qw) & mask_qword_negative) != 0;
                }
            }
        }

        void get_operand_value(
            common::result& r,
            const operand_encoding_t& operand,
            register_value_alias_t& value) const;

        bool bounds_check_address(
            common::result& r,
            const register_value_alias_t& address);

        void initialize_allocator();

        void get_address_with_offset(
            common::result& r,
            const instruction_t* inst,
            uint8_t address_index,
            uint8_t offset_index,
            register_value_alias_t& address);

        void set_target_operand_value(
            common::result& r,
            const operand_encoding_t& operand,
            op_sizes size,
            const register_value_alias_t& value);

        void execute_trap(uint8_t index);

        void get_constant_address_or_pc_with_offset(
            common::result& r,
            const instruction_t* inst,
            uint8_t operand_index,
            uint64_t inst_size,
            register_value_alias_t& address);

    private:
        ffi* _ffi = nullptr;
        bool _exited = false;
        size_t _heap_size = 0;
        size_t _stack_size = 0;
        uint8_t* _heap = nullptr;
        instruction_cache _icache;
        uint64_t _heap_address = 0;
        register_file_t _registers {};
        allocator* _allocator = nullptr;
        meta_information_t _meta_information {};
        std::unordered_map<uint8_t, trap_callable> _traps {};
    };

}
