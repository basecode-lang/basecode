#pragma once

#include <cstdint>
#include "result.h"
#include "terp.h"

namespace basecode {

    class terp;

    class instruction_emitter {
    public:
        explicit instruction_emitter(terp* t);

        bool rts(result& r);

        bool nop(result& r);

        bool exit(result& r);

        uint64_t location_counter() const;

        bool add_int_register_to_register(
            result& r,
            op_sizes size,
            uint8_t target_index,
            uint8_t lhs_index,
            uint8_t rhs_index);

        bool move_int_constant_to_register(
            result& r,
            op_sizes size,
            uint64_t value,
            uint8_t index);

        bool load_stack_offset_to_register(
            result& r,
            uint8_t target_index,
            uint64_t offset);

        bool store_register_to_stack_offset(
            result& r,
            uint8_t source_index,
            uint64_t offset);

        bool multiply_int_register_to_register(
            result& r,
            op_sizes size,
            uint8_t target_index,
            uint8_t lhs_index,
            uint8_t rhs_index);

        void location_counter(uint64_t value);

        bool jump_direct(result& r, uint64_t address);

        bool push_float_constant(result& r, double value);

        bool pop_float_register(result& r, uint8_t index);

        bool jump_subroutine_indirect(result& r, uint8_t index);

        bool jump_subroutine_direct(result& r, uint64_t address);

        bool pop_int_register(result& r, op_sizes size, uint8_t index);

        bool push_int_register(result& r, op_sizes size, uint8_t index);

        bool push_int_constant(result& r, op_sizes size, uint64_t value);

    private:
        terp* _terp = nullptr;
        uint64_t _location_counter = 0;
    };

};