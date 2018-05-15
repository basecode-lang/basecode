#include <chrono>
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
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    basecode::terp terp((1024 * 1024) * 32);
    basecode::result r;
    if (!terp.initialize(r)) {
        fmt::print("terp initialize failed.\n");
        print_results(r);
        return 1;
    }

    basecode::instruction_emitter bootstrap_emitter(0);
    bootstrap_emitter.jump_direct(0);

    basecode::instruction_emitter fn_square_emitter(bootstrap_emitter.end_address());
    fn_square_emitter.load_stack_offset_to_register(0, 8);
    fn_square_emitter.multiply_int_register_to_register(basecode::op_sizes::dword, 0, 0, 0);
    fn_square_emitter.store_register_to_stack_offset(0, 8);
    fn_square_emitter.rts();

    basecode::instruction_emitter main_emitter(fn_square_emitter.end_address());
    main_emitter.push_int_constant(basecode::op_sizes::dword, 9);
    main_emitter.jump_subroutine_direct(fn_square_emitter.start_address());
    main_emitter.pop_int_register(basecode::op_sizes::dword, 5);
    main_emitter.push_int_constant(basecode::op_sizes::dword, 5);
    main_emitter.jump_subroutine_direct(fn_square_emitter.start_address());
    main_emitter.pop_int_register(basecode::op_sizes::dword, 6);
    main_emitter.exit();

    bootstrap_emitter[0].patch_branch_address(main_emitter.start_address());
    bootstrap_emitter.encode(r, terp);
    fn_square_emitter.encode(r, terp);
    main_emitter.encode(r, terp);

    if (r.is_failed()) {
        print_results(r);
        return 1;
    }

    terp.dump_heap(0);
    fmt::print("Disassembly:\n{}\n", terp.disassemble(r, 0));

    while (!terp.has_exited()) {
        if (!terp.step(r)) {
            print_results(r);
            return 1;
        }
    }
    terp.dump_state();

    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    fmt::print("execution time (in us): {}", duration);

    return 0;
}