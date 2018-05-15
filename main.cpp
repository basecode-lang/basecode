#include <chrono>
#include <iostream>
#include <functional>
#include <fmt/format.h>
#include "terp.h"
#include "hex_formatter.h"
#include "instruction_emitter.h"

using test_function_callable = std::function<bool (basecode::result&, basecode::terp&)>;

static void print_results(basecode::result& r) {
    fmt::print("result success: {}\n", !r.is_failed());
    for (const auto& msg : r.messages()) {
        fmt::print(
            "\t|{}|{}{}\n",
            msg.code(),
            msg.is_error() ? "ERROR: " : " ",
            msg.message());
    }
}

static bool run_terp(basecode::result& r, basecode::terp& terp) {
    while (!terp.has_exited())
        if (!terp.step(r))
            return false;

    return true;
}

static bool test_square(basecode::result& r, basecode::terp& terp) {
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

    if (r.is_failed())
        return false;

    auto result = run_terp(r, terp);
    if (terp.register_file().i[5] != 81) {
        r.add_message("T001", "I5 should contain 81.", true);
    }

    if (terp.register_file().i[6] != 25) {
        r.add_message("T001", "I6 should contain 25.", true);
    }

    return result;
}

static bool test_fibonacci(basecode::result& r, basecode::terp& terp) {
    basecode::instruction_emitter bootstrap_emitter(0);
    bootstrap_emitter.jump_direct(0);

    basecode::instruction_emitter fn_fibonacci(bootstrap_emitter.end_address());
    fn_fibonacci.load_stack_offset_to_register(0, 8);
    fn_fibonacci.compare_int_register_to_constant(basecode::op_sizes::dword, 0, 0);
    fn_fibonacci.branch_if_equal(0);
    fn_fibonacci.compare_int_register_to_constant(basecode::op_sizes::dword, 0, 1);
    fn_fibonacci.branch_if_equal(0);
    fn_fibonacci.jump_direct(0);
    auto label_exit_fib = fn_fibonacci.end_address();
    fn_fibonacci[2].patch_branch_address(label_exit_fib);
    fn_fibonacci[4].patch_branch_address(label_exit_fib);
    fn_fibonacci.rts();

    auto label_next_fib = fn_fibonacci.end_address();
    fn_fibonacci[5].patch_branch_address(label_next_fib);

    fn_fibonacci.subtract_int_constant_from_register(basecode::op_sizes::dword, 1, 0, 1);
    fn_fibonacci.subtract_int_constant_from_register(basecode::op_sizes::dword, 2, 0, 2);
    fn_fibonacci.push_int_register(basecode::op_sizes::dword, 2);
    fn_fibonacci.jump_subroutine_direct(fn_fibonacci.start_address());
    fn_fibonacci.pop_int_register(basecode::op_sizes::dword, 2);
    fn_fibonacci.add_int_register_to_register(basecode::op_sizes::dword, 1, 1, 2);
    fn_fibonacci.push_int_register(basecode::op_sizes::dword, 1);
    fn_fibonacci.jump_subroutine_direct(fn_fibonacci.start_address());
    fn_fibonacci.pop_int_register(basecode::op_sizes::dword, 1);
    fn_fibonacci.store_register_to_stack_offset(1, 8);
    fn_fibonacci.rts();

    basecode::instruction_emitter main_emitter(fn_fibonacci.end_address());
    main_emitter.push_int_constant(basecode::op_sizes::dword, 100);
    main_emitter.jump_subroutine_direct(fn_fibonacci.start_address());
    main_emitter.pop_int_register(basecode::op_sizes::dword, 0);
    main_emitter.exit();

    bootstrap_emitter[0].patch_branch_address(main_emitter.start_address());
    bootstrap_emitter.encode(r, terp);
    fn_fibonacci.encode(r, terp);
    main_emitter.encode(r, terp);

    auto result = run_terp(r, terp);
    //terp.dump_state(2);

    if (terp.register_file().i[0] != 1) {
        r.add_message("T001", "fn_fibonacci should end with 1 or 0.", true);
    }

    return result;
}

static int time_test_function(
        basecode::result& r,
        basecode::terp& terp,
        const std::string& title,
        const test_function_callable& test_function) {
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    terp.reset();
    auto rc = test_function(r, terp);
    //fmt::print("\nASSEMBLY LISTING:\n{}\n", terp.disassemble(r, 0));
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    fmt::print("function: {} {}\n", title, rc ? "SUCCESS" : "FAILED");
    if (!rc || r.is_failed()) {
        print_results(r);
    }
    fmt::print("execution time (micro seconds): {}\n\n", duration);

    return rc;
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

    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    fmt::print("terp startup time (in microseconds): {}\n\n", duration);

    time_test_function(r, terp, "test_square", test_square);
    time_test_function(r, terp, "test_fibonacci", test_fibonacci);

    return 0;
}