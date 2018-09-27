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
#include <unordered_map>
#include <common/result.h>
#include <boost/filesystem.hpp>
#include "vm_types.h"

namespace basecode::vm {

    struct icache_entry_t {
        size_t size;
        instruction_t inst;
    };

    class instruction_cache {
    public:
        explicit instruction_cache(terp* terp);

        void reset();

        size_t fetch_at(
            common::result& r,
            uint64_t address,
            instruction_t& inst);

        size_t fetch(common::result& r, instruction_t& inst);

    private:
        terp* _terp = nullptr;
        std::unordered_map<uint64_t, icache_entry_t> _cache {};
    };

    ///////////////////////////////////////////////////////////////////////////

    class terp {
    public:
        using trap_callable = std::function<void (terp*)>;

        static constexpr uint64_t mask_byte_negative  = 0b0000000000000000000000000000000000000000000000000000000010000000;
        static constexpr uint64_t mask_word_negative  = 0b0000000000000000000000000000000000000000000000001000000000000000;
        static constexpr uint64_t mask_dword_negative = 0b0000000000000000000000000000000010000000000000000000000000000000;
        static constexpr uint64_t mask_qword_negative = 0b1000000000000000000000000000000000000000000000000000000000000000;

        static constexpr size_t interrupt_vector_table_start = 0;
        static constexpr size_t interrupt_vector_table_size = 16;
        static constexpr size_t interrupt_vector_table_end = interrupt_vector_table_start +
            (sizeof(uint64_t) * interrupt_vector_table_size);

        static constexpr size_t heap_vector_table_start = interrupt_vector_table_end;
        static constexpr size_t heap_vector_table_size = 16;
        static constexpr size_t heap_vector_table_end = heap_vector_table_start
            + (sizeof(uint16_t) * heap_vector_table_size);

        static constexpr size_t program_start = heap_vector_table_end;

        static constexpr uint8_t trap_out_of_memory = 0xff;
        static constexpr uint8_t trap_invalid_ffi_call = 0xfe;

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

        bool step(common::result& r);

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
        bool has_overflow(
            uint64_t lhs,
            uint64_t rhs,
            uint64_t result,
            op_sizes size);

        void set_zoned_value(
            register_type_t type,
            register_value_alias_t& reg,
            uint64_t value,
            op_sizes size);

        bool get_operand_value(
            common::result& r,
            const instruction_t& inst,
            uint8_t operand_index,
            operand_value_t& value) const;

        bool set_target_operand_value(
            common::result& r,
            const operand_encoding_t& operand,
            op_sizes size,
            const operand_value_t& value);

        void execute_trap(uint8_t index);

        bool get_constant_address_or_pc_with_offset(
            common::result& r,
            const instruction_t& inst,
            uint8_t operand_index,
            uint64_t inst_size,
            operand_value_t& address);

        bool has_carry(const operand_value_t& value, op_sizes size);

        bool is_negative(const operand_value_t& value, op_sizes size);

    private:
        ffi* _ffi = nullptr;
        bool _exited = false;
        size_t _heap_size = 0;
        size_t _stack_size = 0;
        uint8_t* _heap = nullptr;
        instruction_cache _icache;
        register_file_t _registers {};
        allocator* _allocator = nullptr;
        meta_information_t _meta_information {};
        std::unordered_map<uint8_t, trap_callable> _traps {};
    };

};
