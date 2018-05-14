#include "terp.h"
#include "instruction_emitter.h"

namespace basecode {

    instruction_emitter::instruction_emitter(terp* t) : _terp(t) {
    }

    bool instruction_emitter::nop(result& r) {
        basecode::instruction_t no_op;
        no_op.op = basecode::op_codes::nop;

        auto inst_size = _terp->encode_instruction(r, _location_counter, no_op);
        if (inst_size == 0)
            return false;

        _location_counter += inst_size;

        return true;
    }

    bool instruction_emitter::rts(result& r) {
        basecode::instruction_t rts_op;
        rts_op.op = basecode::op_codes::rts;

        auto inst_size = _terp->encode_instruction(r, _location_counter, rts_op);
        if (inst_size == 0)
            return false;

        _location_counter += inst_size;

        return true;
    }

    bool instruction_emitter::exit(result& r) {
        basecode::instruction_t exit_op;
        exit_op.op = basecode::op_codes::exit;

        auto inst_size = _terp->encode_instruction(r, _location_counter, exit_op);
        if (inst_size == 0)
            return false;

        _location_counter += inst_size;
        return true;
    }

    bool instruction_emitter::add_int_register_to_register(
            result& r,
            op_sizes size,
            uint8_t target_index,
            uint8_t lhs_index,
            uint8_t rhs_index) {
        basecode::instruction_t add_op;
        add_op.op = basecode::op_codes::add;
        add_op.size = size;
        add_op.operands_count = 3;
        add_op.operands[0].type = basecode::operand_types::register_integer;
        add_op.operands[0].index = target_index;
        add_op.operands[1].type = basecode::operand_types::register_integer;
        add_op.operands[1].index = lhs_index;
        add_op.operands[2].type = basecode::operand_types::register_integer;
        add_op.operands[2].index = rhs_index;

        auto inst_size = _terp->encode_instruction(r, _location_counter, add_op);
        if (inst_size == 0)
            return false;

        _location_counter += inst_size;

        return true;
    }

    bool instruction_emitter::load_stack_offset_to_register(
            result& r,
            uint8_t target_index,
            uint64_t offset) {
        basecode::instruction_t load_op;
        load_op.op = basecode::op_codes::load;
        load_op.size = basecode::op_sizes::qword;
        load_op.operands_count = 3;
        load_op.operands[0].type = basecode::operand_types::register_integer;
        load_op.operands[0].index = target_index;
        load_op.operands[1].type = basecode::operand_types::register_sp;
        load_op.operands[1].index = 0;
        load_op.operands[2].type = basecode::operand_types::constant_integer;
        load_op.operands[2].value.u64 = offset;

        auto inst_size = _terp->encode_instruction(r, _location_counter, load_op);
        if (inst_size == 0)
            return false;

        _location_counter += inst_size;

        return true;
    }

    bool instruction_emitter::store_register_to_stack_offset(
            result& r,
            uint8_t source_index,
            uint64_t offset) {
        basecode::instruction_t store_op;
        store_op.op = basecode::op_codes::store;
        store_op.size = basecode::op_sizes::qword;
        store_op.operands_count = 3;
        store_op.operands[0].type = basecode::operand_types::register_integer;
        store_op.operands[0].index = source_index;
        store_op.operands[1].type = basecode::operand_types::register_sp;
        store_op.operands[2].type = basecode::operand_types::constant_integer;
        store_op.operands[2].value.u64 = offset;

        auto inst_size = _terp->encode_instruction(r, _location_counter, store_op);
        if (inst_size == 0)
            return false;

        _location_counter += inst_size;

        return true;
    }

    bool instruction_emitter::multiply_int_register_to_register(
            result& r,
            op_sizes size,
            uint8_t target_index,
            uint8_t lhs_index,
            uint8_t rhs_index) {
        basecode::instruction_t mul_op;
        mul_op.op = basecode::op_codes::mul;
        mul_op.size = size;
        mul_op.operands_count = 3;
        mul_op.operands[0].index = target_index;
        mul_op.operands[0].type = basecode::operand_types::register_integer;
        mul_op.operands[1].index = lhs_index;
        mul_op.operands[1].type = basecode::operand_types::register_integer;
        mul_op.operands[2].index = rhs_index;
        mul_op.operands[2].type = basecode::operand_types::register_integer;
        auto inst_size = _terp->encode_instruction(r, _location_counter, mul_op);
        if (inst_size == 0) {
            return false;
        }
        _location_counter += inst_size;
        return true;
    }

    bool instruction_emitter::move_int_constant_to_register(
            result& r,
            op_sizes size,
            uint64_t value,
            uint8_t index) {
        basecode::instruction_t move_op;
        move_op.op = basecode::op_codes::move;
        move_op.size = size;
        move_op.operands_count = 2;
        move_op.operands[0].value.u64 = value;
        move_op.operands[0].type = basecode::operand_types::constant_integer;
        move_op.operands[1].index = index;
        move_op.operands[1].type = basecode::operand_types::register_integer;

        auto inst_size = _terp->encode_instruction(r, _location_counter, move_op);
        if (inst_size == 0)
            return false;

        _location_counter += inst_size;

        return true;
    }

    uint64_t instruction_emitter::location_counter() const {
        return _location_counter;
    }

    void instruction_emitter::location_counter(uint64_t value) {
        _location_counter = value;
    }

    bool instruction_emitter::jump_direct(result& r, uint64_t address) {
        basecode::instruction_t jmp_op;
        jmp_op.op = basecode::op_codes::jmp;
        jmp_op.size = basecode::op_sizes::qword;
        jmp_op.operands_count = 1;
        jmp_op.operands[0].type = basecode::operand_types::constant_integer;
        jmp_op.operands[0].value.u64 = address;

        auto inst_size = _terp->encode_instruction(r, _location_counter, jmp_op);
        if (inst_size == 0)
            return false;

        _location_counter += inst_size;

        return true;
    }

    bool instruction_emitter::pop_float_register(result& r, uint8_t index) {
        basecode::instruction_t pop_op;
        pop_op.op = basecode::op_codes::pop;
        pop_op.operands_count = 1;
        pop_op.operands[0].type = basecode::operand_types::register_floating_point;
        pop_op.operands[0].index = index;

        auto inst_size = _terp->encode_instruction(r, _location_counter, pop_op);
        if (inst_size == 0)
            return false;

        _location_counter += inst_size;

        return true;
    }

    bool instruction_emitter::push_float_constant(result& r, double value) {
        basecode::instruction_t push_op;
        push_op.op = basecode::op_codes::push;
        push_op.operands_count = 1;
        push_op.operands[0].type = basecode::operand_types::constant_float;
        push_op.operands[0].value.d64 = value;

        auto inst_size = _terp->encode_instruction(r, _location_counter, push_op);
        if (inst_size == 0)
            return false;

        _location_counter += inst_size;

        return true;
    }

    bool instruction_emitter::jump_subroutine_indirect(result& r, uint8_t index) {
        basecode::instruction_t jsr_op;
        jsr_op.op = basecode::op_codes::jsr;
        jsr_op.size = basecode::op_sizes::qword;
        jsr_op.operands_count = 1;
        jsr_op.operands[0].type = basecode::operand_types::register_integer;
        jsr_op.operands[0].index = index;

        auto inst_size = _terp->encode_instruction(r, _location_counter, jsr_op);
        if (inst_size == 0)
            return false;

        _location_counter += inst_size;

        return true;
    }

    bool instruction_emitter::jump_subroutine_direct(result& r, uint64_t address) {
        basecode::instruction_t jsr_op;
        jsr_op.op = basecode::op_codes::jsr;
        jsr_op.size = basecode::op_sizes::qword;
        jsr_op.operands_count = 1;
        jsr_op.operands[0].type = basecode::operand_types::constant_integer;
        jsr_op.operands[0].value.u64 = address;

        auto inst_size = _terp->encode_instruction(r, _location_counter, jsr_op);
        if (inst_size == 0)
            return false;

        _location_counter += inst_size;

        return true;
    }

    bool instruction_emitter::pop_int_register(result& r, op_sizes size, uint8_t index) {
        basecode::instruction_t pop_op;
        pop_op.op = basecode::op_codes::pop;
        pop_op.size = size;
        pop_op.operands_count = 1;
        pop_op.operands[0].type = basecode::operand_types::register_integer;
        pop_op.operands[0].index = index;

        auto inst_size = _terp->encode_instruction(r, _location_counter, pop_op);
        if (inst_size == 0)
            return false;

        _location_counter += inst_size;

        return true;
    }

    bool instruction_emitter::push_int_register(result& r, op_sizes size, uint8_t index) {
        basecode::instruction_t push_op;
        push_op.op = basecode::op_codes::push;
        push_op.size = size;
        push_op.operands_count = 1;
        push_op.operands[0].type = basecode::operand_types::register_integer;
        push_op.operands[0].index = index;

        auto inst_size = _terp->encode_instruction(r, _location_counter, push_op);
        if (inst_size == 0)
            return false;

        _location_counter += inst_size;

        return true;
    }

    bool instruction_emitter::push_int_constant(result& r, op_sizes size, uint64_t value) {
        basecode::instruction_t push;
        push.op = basecode::op_codes::push;
        push.size = size;
        push.operands_count = 1;
        push.operands[0].type = basecode::operand_types::constant_integer;
        push.operands[0].value.u64 = value;

        auto inst_size = _terp->encode_instruction(r, _location_counter, push);
        if (inst_size == 0)
            return false;

        _location_counter += inst_size;

        return true;
    }

};