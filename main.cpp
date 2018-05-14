#include <iostream>
#include <fmt/format.h>
#include "terp.h"
#include "hex_formatter.h"
#include "instruction_emitter.h"

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

    basecode::instruction_emitter emitter(&terp);
    emitter.nop(r);
    emitter.jump_direct(r, 1024);

    auto fn_square = emitter.location_counter();
    emitter.load_stack_offset_to_register(r, 0, 8);
    emitter.multiply_int_register_to_register(r, basecode::op_sizes::dword, 0, 0, 0);
    emitter.store_register_to_stack_offset(r, 0, 8);
    emitter.rts(r);

    emitter.location_counter(1024);
    emitter.nop(r);
    emitter.push_int_constant(r, basecode::op_sizes::dword, 9);
    emitter.jump_subroutine_direct(r, fn_square);
    emitter.pop_int_register(r, basecode::op_sizes::dword, 5);

    emitter.push_int_constant(r, basecode::op_sizes::dword, 5);
    emitter.jump_subroutine_direct(r, fn_square);
    emitter.pop_int_register(r, basecode::op_sizes::dword, 6);

    emitter.exit(r);

    if (r.is_failed()) {
        print_results(r);
        return 1;
    }

    terp.dump_heap(0);
    terp.dump_heap(1024);

    while (!terp.has_exited()) {
        if (!terp.step(r)) {
            print_results(r);
            return 1;
        }
        terp.dump_state();
    }

    return 0;
}