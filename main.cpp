#include <fmt/format.h>
#include <iostream>
#include "terp.h"
#include "hex_formatter.h"

void print_results(basecode::result& r) {
    fmt::print("result success: {}\n", !r.is_failed());
    for (const auto& msg : r.messages()) {
        fmt::print(
            "\t{} code: {}| message: {}\n",
            msg.is_error() ? "ERROR" : "",
            msg.code(),
            msg.message());
    }
}

int main() {
    basecode::terp terp((1024 * 1024) * 32);

    basecode::result r;
    if (!terp.initialize(r)) {
        fmt::print("terp initialize failed.\n");
        print_results(r);
        return 1;
    }

    fmt::print(
        "heap size: {} bytes, {} qwords\n",
        terp.heap_size(),
        terp.heap_size_in_qwords());

    const auto& regs = terp.register_file();
    auto stack_top = regs.sp;
    fmt::print("regs.sp = ${:08x}\n", regs.sp);

    terp.push(0x08);
    terp.push(0x04);
    terp.push(0x02);

    fmt::print(
        "regs.sp = ${:08x}, number of entries: {}\n",
        regs.sp,
        (stack_top - regs.sp) / sizeof(uint64_t));

    auto t1 = terp.pop();
    auto t2 = terp.pop();
    auto t3 = terp.pop();

    fmt::print("t1 = ${:08x}\n", t1);
    fmt::print("t2 = ${:08x}\n", t2);
    fmt::print("t2 = ${:08x}\n", t3);

    size_t inst_size = 0;
    uint64_t location_counter = 0;

    basecode::instruction_t nop;
    nop.op = basecode::op_codes::nop;
    nop.operands_count = 0;
    inst_size = terp.encode_instruction(r, location_counter, nop);
    if (inst_size == 0) {
        print_results(r);
        return 1;
    }
    location_counter += inst_size;

    basecode::instruction_t move_constant_to_reg3;
    move_constant_to_reg3.op = basecode::op_codes::move;
    move_constant_to_reg3.size = basecode::op_sizes::byte;
    move_constant_to_reg3.operands_count = 2;
    move_constant_to_reg3.operands[0].value = 0x7f;
    move_constant_to_reg3.operands[0].type = basecode::operand_types::constant;
    move_constant_to_reg3.operands[1].index = 3;
    move_constant_to_reg3.operands[1].type = basecode::operand_types::register_integer;
    inst_size = terp.encode_instruction(r, location_counter, move_constant_to_reg3);
    if (inst_size == 0) {
        print_results(r);
        return 1;
    }
    location_counter += inst_size;

    basecode::instruction_t move_constant_to_reg4;
    move_constant_to_reg4.op = basecode::op_codes::move;
    move_constant_to_reg4.size = basecode::op_sizes::byte;
    move_constant_to_reg4.operands_count = 2;
    move_constant_to_reg4.operands[0].value = 0x7f;
    move_constant_to_reg4.operands[0].type = basecode::operand_types::constant;
    move_constant_to_reg4.operands[1].index = 4;
    move_constant_to_reg4.operands[1].type = basecode::operand_types::register_integer;
    inst_size = terp.encode_instruction(r, location_counter, move_constant_to_reg4);
    if (inst_size == 0) {
        print_results(r);
        return 1;
    }
    location_counter += inst_size;

    basecode::instruction_t add_i3_to_i4;
    add_i3_to_i4.op = basecode::op_codes::add;
    add_i3_to_i4.size = basecode::op_sizes::byte;
    add_i3_to_i4.operands_count = 3;
    add_i3_to_i4.operands[0].index = 2;
    add_i3_to_i4.operands[0].type = basecode::operand_types::register_integer;
    add_i3_to_i4.operands[1].index = 3;
    add_i3_to_i4.operands[1].type = basecode::operand_types::register_integer;
    add_i3_to_i4.operands[2].index = 4;
    add_i3_to_i4.operands[2].type = basecode::operand_types::register_integer;
    inst_size = terp.encode_instruction(r, location_counter, add_i3_to_i4);
    if (inst_size == 0) {
        print_results(r);
        return 1;
    }
    location_counter += inst_size;

    terp.dump_heap(0);

    for (size_t steps = 0; steps < 4; steps++) {
        if (!terp.step(r)) {
            print_results(r);
            return 1;
        }
        terp.dump_state();
    }

    return 0;
}