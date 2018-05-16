#include "terp.h"
#include "instruction_emitter.h"

namespace basecode {

    instruction_emitter::instruction_emitter(
            uint64_t address) : _start_address(address) {
    }

    void instruction_emitter::nop() {
        basecode::instruction_t no_op;
        no_op.op = basecode::op_codes::nop;
        _instructions.push_back(no_op);
    }

    void instruction_emitter::rts() {
        basecode::instruction_t rts_op;
        rts_op.op = basecode::op_codes::rts;
        _instructions.push_back(rts_op);
    }

    void instruction_emitter::exit() {
        basecode::instruction_t exit_op;
        exit_op.op = basecode::op_codes::exit;
        _instructions.push_back(exit_op);
    }

    size_t instruction_emitter::size() const {
        size_t size = 0;
        for (const auto& inst : _instructions)
            size += inst.encoding_size();
        return size;
    }

    uint64_t instruction_emitter::end_address() const {
        return _start_address + size();
    }

    uint64_t instruction_emitter::start_address() const {
        return _start_address;
    }

    void instruction_emitter::load_with_offset_to_register(
            op_sizes size,
            uint8_t source_index,
            uint8_t target_index,
            uint64_t offset) {
        basecode::instruction_t load_op;
        load_op.op = basecode::op_codes::load;
        load_op.size = size;
        load_op.operands_count = 3;
        load_op.operands[0].type = basecode::operand_types::register_integer;
        load_op.operands[0].index = target_index;
        load_op.operands[1].type = basecode::operand_types::register_integer;
        load_op.operands[1].index = source_index;
        load_op.operands[2].type = basecode::operand_types::constant_integer;
        load_op.operands[2].value.u64 = offset;
        _instructions.push_back(load_op);
    }

    void instruction_emitter::add_int_register_to_register(
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
        _instructions.push_back(add_op);
    }

    void instruction_emitter::load_stack_offset_to_register(
            op_sizes size,
            uint8_t target_index,
            uint64_t offset) {
        basecode::instruction_t load_op;
        load_op.op = basecode::op_codes::load;
        load_op.size = size;
        load_op.operands_count = 3;
        load_op.operands[0].type = basecode::operand_types::register_integer;
        load_op.operands[0].index = target_index;
        load_op.operands[1].type = basecode::operand_types::register_sp;
        load_op.operands[1].index = 0;
        load_op.operands[2].type = basecode::operand_types::constant_integer;
        load_op.operands[2].value.u64 = offset;
        _instructions.push_back(load_op);
    }

    void instruction_emitter::store_register_to_stack_offset(
            op_sizes size,
            uint8_t source_index,
            uint64_t offset) {
        basecode::instruction_t store_op;
        store_op.op = basecode::op_codes::store;
        store_op.size = size;
        store_op.operands_count = 3;
        store_op.operands[0].type = basecode::operand_types::register_integer;
        store_op.operands[0].index = source_index;
        store_op.operands[1].type = basecode::operand_types::register_sp;
        store_op.operands[2].type = basecode::operand_types::constant_integer;
        store_op.operands[2].value.u64 = offset;
        _instructions.push_back(store_op);
    }

    void instruction_emitter::divide_int_register_to_register(
            op_sizes size,
            uint8_t target_index,
            uint8_t lhs_index,
            uint8_t rhs_index) {
        basecode::instruction_t div_op;
        div_op.op = basecode::op_codes::div;
        div_op.size = size;
        div_op.operands_count = 3;
        div_op.operands[0].type = basecode::operand_types::register_integer;
        div_op.operands[0].index = target_index;
        div_op.operands[1].type = basecode::operand_types::register_integer;
        div_op.operands[1].index = lhs_index;
        div_op.operands[2].type = basecode::operand_types::register_integer;
        div_op.operands[2].index = rhs_index;
        _instructions.push_back(div_op);
    }

    bool instruction_emitter::encode(result& r, terp& terp) {
        size_t offset = 0;
        for (auto& inst : _instructions) {
            auto inst_size = inst.encode(r, terp.heap(), _start_address + offset);
            if (inst_size == 0)
                return false;
            offset += inst_size;
        }
        return true;
    }

    void instruction_emitter::store_with_offset_from_register(
            op_sizes size,
            uint8_t source_index,
            uint8_t target_index,
            uint64_t offset) {
        basecode::instruction_t store_op;
        store_op.op = basecode::op_codes::store;
        store_op.size = size;
        store_op.operands_count = 3;
        store_op.operands[0].type = basecode::operand_types::register_integer;
        store_op.operands[0].index = source_index;
        store_op.operands[1].type = basecode::operand_types::register_integer;
        store_op.operands[1].index = target_index;
        store_op.operands[2].type = basecode::operand_types::constant_integer;
        store_op.operands[2].value.u64 = offset;
        _instructions.push_back(store_op);
    }

    void instruction_emitter::subtract_int_register_to_register(
            op_sizes size,
            uint8_t target_index,
            uint8_t lhs_index,
            uint8_t rhs_index) {
        basecode::instruction_t sub_op;
        sub_op.op = basecode::op_codes::sub;
        sub_op.size = size;
        sub_op.operands_count = 3;
        sub_op.operands[0].type = basecode::operand_types::register_integer;
        sub_op.operands[0].index = target_index;
        sub_op.operands[1].type = basecode::operand_types::register_integer;
        sub_op.operands[1].index = lhs_index;
        sub_op.operands[2].type = basecode::operand_types::register_integer;
        sub_op.operands[2].index = rhs_index;
        _instructions.push_back(sub_op);
    }

    void instruction_emitter::subtract_int_constant_from_register(
            op_sizes size,
            uint8_t target_index,
            uint8_t lhs_index,
            uint64_t rhs_value) {
        basecode::instruction_t sub_op;
        sub_op.op = basecode::op_codes::sub;
        sub_op.size = size;
        sub_op.operands_count = 3;
        sub_op.operands[0].type = basecode::operand_types::register_integer;
        sub_op.operands[0].index = target_index;
        sub_op.operands[1].type = basecode::operand_types::register_integer;
        sub_op.operands[1].index = lhs_index;
        sub_op.operands[2].type = basecode::operand_types::constant_integer;
        sub_op.operands[2].value.u64 = rhs_value;
        _instructions.push_back(sub_op);
    }

    void instruction_emitter::multiply_int_register_to_register(
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
        _instructions.push_back(mul_op);
    }

    void instruction_emitter::move_int_constant_to_register(
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
        _instructions.push_back(move_op);
    }

    void instruction_emitter::jump_direct(uint64_t address) {
        basecode::instruction_t jmp_op;
        jmp_op.op = basecode::op_codes::jmp;
        jmp_op.operands_count = 1;
        jmp_op.operands[0].type = basecode::operand_types::constant_integer;
        jmp_op.operands[0].value.u64 = address;
        _instructions.push_back(jmp_op);
    }

    void instruction_emitter::pop_float_register(uint8_t index) {
        basecode::instruction_t pop_op;
        pop_op.op = basecode::op_codes::pop;
        pop_op.operands_count = 1;
        pop_op.operands[0].type = basecode::operand_types::register_floating_point;
        pop_op.operands[0].index = index;
        _instructions.push_back(pop_op);
    }

    void instruction_emitter::push_float_constant(double value) {
        basecode::instruction_t push_op;
        push_op.op = basecode::op_codes::push;
        push_op.operands_count = 1;
        push_op.operands[0].type = basecode::operand_types::constant_float;
        push_op.operands[0].value.d64 = value;
        _instructions.push_back(push_op);
    }

    void instruction_emitter::dec(op_sizes size, uint8_t index) {
        basecode::instruction_t dec_op;
        dec_op.op = basecode::op_codes::dec;
        dec_op.operands_count = 1;
        dec_op.operands[0].type = basecode::operand_types::register_integer;
        dec_op.operands[0].index = index;
        _instructions.push_back(dec_op);
    }

    void instruction_emitter::inc(op_sizes size, uint8_t index) {
        basecode::instruction_t inc_op;
        inc_op.op = basecode::op_codes::inc;
        inc_op.operands_count = 1;
        inc_op.operands[0].type = basecode::operand_types::register_integer;
        inc_op.operands[0].index = index;
        _instructions.push_back(inc_op);
    }

    void instruction_emitter::jump_subroutine_indirect(uint8_t index) {
        basecode::instruction_t jsr_op;
        jsr_op.op = basecode::op_codes::jsr;
        jsr_op.operands_count = 1;
        jsr_op.operands[0].type = basecode::operand_types::register_integer;
        jsr_op.operands[0].index = index;
        _instructions.push_back(jsr_op);
    }

    void instruction_emitter::jump_subroutine_direct(uint64_t address) {
        basecode::instruction_t jsr_op;
        jsr_op.op = basecode::op_codes::jsr;
        jsr_op.operands_count = 1;
        jsr_op.operands[0].type = basecode::operand_types::constant_integer;
        jsr_op.operands[0].value.u64 = address;
        _instructions.push_back(jsr_op);
    }

    void instruction_emitter::pop_int_register(op_sizes size, uint8_t index) {
        basecode::instruction_t pop_op;
        pop_op.op = basecode::op_codes::pop;
        pop_op.size = size;
        pop_op.operands_count = 1;
        pop_op.operands[0].type = basecode::operand_types::register_integer;
        pop_op.operands[0].index = index;
        _instructions.push_back(pop_op);
    }

    void instruction_emitter::push_int_register(op_sizes size, uint8_t index) {
        basecode::instruction_t push_op;
        push_op.op = basecode::op_codes::push;
        push_op.size = size;
        push_op.operands_count = 1;
        push_op.operands[0].type = basecode::operand_types::register_integer;
        push_op.operands[0].index = index;
        _instructions.push_back(push_op);
    }

    void instruction_emitter::push_int_constant(op_sizes size, uint64_t value) {
        basecode::instruction_t push_op;
        push_op.op = basecode::op_codes::push;
        push_op.size = size;
        push_op.operands_count = 1;
        push_op.operands[0].type = basecode::operand_types::constant_integer;
        push_op.operands[0].value.u64 = value;
        _instructions.push_back(push_op);
    }

    void instruction_emitter::compare_int_register_to_register(
            op_sizes size,
            uint8_t lhs_index,
            uint8_t rhs_index) {
        basecode::instruction_t cmp_op;
        cmp_op.op = basecode::op_codes::cmp;
        cmp_op.size = size;
        cmp_op.operands_count = 2;
        cmp_op.operands[0].type = basecode::operand_types::register_integer;
        cmp_op.operands[0].index = lhs_index;
        cmp_op.operands[1].type = basecode::operand_types::register_integer;
        cmp_op.operands[1].index = rhs_index;
        _instructions.push_back(cmp_op);
    }

    void instruction_emitter::compare_int_register_to_constant(
            op_sizes size,
            uint8_t index,
            uint64_t value) {
        basecode::instruction_t cmp_op;
        cmp_op.op = basecode::op_codes::cmp;
        cmp_op.size = size;
        cmp_op.operands_count = 2;
        cmp_op.operands[0].type = basecode::operand_types::register_integer;
        cmp_op.operands[0].index = index;
        cmp_op.operands[1].type = basecode::operand_types::constant_integer;
        cmp_op.operands[1].value.u64 = value;
        _instructions.push_back(cmp_op);
    }

    void instruction_emitter::branch_if_equal(uint64_t address) {
        basecode::instruction_t branch_op;
        branch_op.op = basecode::op_codes::beq;
        branch_op.operands_count = 1;
        branch_op.operands[0].type = basecode::operand_types::constant_integer;
        branch_op.operands[0].value.u64 = address;
        _instructions.push_back(branch_op);
    }

    void instruction_emitter::branch_if_not_equal(uint64_t address) {
        basecode::instruction_t branch_op;
        branch_op.op = basecode::op_codes::bne;
        branch_op.operands_count = 1;
        branch_op.operands[0].type = basecode::operand_types::constant_integer;
        branch_op.operands[0].value.u64 = address;
        _instructions.push_back(branch_op);
    }

};