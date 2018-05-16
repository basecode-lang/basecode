#pragma once

#include <cstdint>
#include "terp.h"
#include "result.h"

namespace basecode {

    class terp;

    class instruction_emitter {
    public:
        explicit instruction_emitter(uint64_t address);

        void rts();

        void nop();

        void exit();

        size_t size() const;

        uint64_t end_address() const;

        uint64_t start_address() const;

        void add_int_register_to_register(
            op_sizes size,
            uint8_t target_index,
            uint8_t lhs_index,
            uint8_t rhs_index);

        void load_with_offset_to_register(
            op_sizes size,
            uint8_t source_index,
            uint8_t target_index,
            uint64_t offset);

        void move_int_constant_to_register(
            op_sizes size,
            uint64_t value,
            uint8_t index);

        bool encode(result& r, terp& terp);

        void load_stack_offset_to_register(
            op_sizes size,
            uint8_t target_index,
            uint64_t offset);

        void store_register_to_stack_offset(
            op_sizes size,
            uint8_t source_index,
            uint64_t offset);

        void store_with_offset_from_register(
            op_sizes size,
            uint8_t source_index,
            uint8_t target_index,
            uint64_t offset);

        void divide_int_register_to_register(
            op_sizes size,
            uint8_t target_index,
            uint8_t lhs_index,
            uint8_t rhs_index);

        void multiply_int_register_to_register(
            op_sizes size,
            uint8_t target_index,
            uint8_t lhs_index,
            uint8_t rhs_index);

        void subtract_int_register_to_register(
            op_sizes size,
            uint8_t target_index,
            uint8_t lhs_index,
            uint8_t rhs_index);

        void subtract_int_constant_from_register(
            op_sizes size,
            uint8_t target_index,
            uint8_t lhs_index,
            uint64_t rhs_value);

        void jump_direct(uint64_t address);

        void push_float_constant(double value);

        void pop_float_register(uint8_t index);

        void dec(op_sizes size, uint8_t index);

        void inc(op_sizes size, uint8_t index);

        void compare_int_register_to_constant(
            op_sizes size,
            uint8_t index,
            uint64_t value);

        void compare_int_register_to_register(
            op_sizes size,
            uint8_t lhs_index,
            uint8_t rhs_index);

        void branch_if_equal(uint64_t address);

        void branch_if_not_equal(uint64_t address);

        void jump_subroutine_indirect(uint8_t index);

        void jump_subroutine_direct(uint64_t address);

        inline instruction_t& operator[](size_t index) {
            return _instructions[index];
        };

        void pop_int_register(op_sizes size, uint8_t index);

        void push_int_register(op_sizes size, uint8_t index);

        void push_int_constant(op_sizes size, uint64_t value);

    private:
        uint64_t _start_address = 0;
        std::vector<instruction_t> _instructions {};
    };

};