#include <chrono>
#include <sstream>
#include <iostream>
#include <functional>
#include <fmt/format.h>
#include "terp.h"
#include "compiler.h"
#include "hex_formatter.h"
#include "instruction_emitter.h"

static constexpr size_t heap_size = (1024 * 1024) * 32;
static constexpr size_t stack_size = (1024 * 1024) * 8;

using test_function_callable = std::function<bool (basecode::result&, basecode::terp&)>;

static void print_results(basecode::result& r) {
    for (const auto& msg : r.messages()) {
        fmt::print(
            "|{}|{}{}\n",
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

static bool test_branches(basecode::result& r, basecode::terp& terp) {
    basecode::instruction_emitter main_emitter(terp.program_start);
    main_emitter.move_int_constant_to_register(basecode::op_sizes::byte, 10, basecode::i_registers_t::i0);
    main_emitter.move_int_constant_to_register(basecode::op_sizes::byte, 5, basecode::i_registers_t::i1);
    main_emitter.compare_int_register_to_register(
        basecode::op_sizes::byte,
        basecode::i_registers_t::i0,
        basecode::i_registers_t::i1);
    main_emitter.branch_if_greater(0);
    main_emitter.push_int_constant(basecode::op_sizes::byte, 1);
    main_emitter.trap(1);

    main_emitter[3].patch_branch_address(main_emitter.end_address());
    main_emitter.compare_int_register_to_register(
        basecode::op_sizes::byte,
        basecode::i_registers_t::i1,
        basecode::i_registers_t::i0);
    main_emitter.branch_if_lesser(0);
    main_emitter.push_int_constant(basecode::op_sizes::byte, 2);
    main_emitter.trap(1);

    main_emitter[7].patch_branch_address(main_emitter.end_address());
    main_emitter.push_int_constant(basecode::op_sizes::byte, 10);
    main_emitter.dup();
    main_emitter.trap(1);
    main_emitter.pop_int_register(basecode::op_sizes::byte, basecode::i_registers_t::i0);

    main_emitter.exit();
    main_emitter.encode(r, terp);

    fmt::print("\nASSEMBLY LISTING:\n{}\n", terp.disassemble(r, terp.program_start));

    auto result = run_terp(r, terp);
    if (terp.register_file().i[0] != 10) {
        r.add_message("T001", "I0 should contain 10.", true);
    }

    return result;
}

static bool test_square(basecode::result& r, basecode::terp& terp) {
    basecode::instruction_emitter bootstrap_emitter(terp.program_start);
    bootstrap_emitter.jump_pc_relative(
        basecode::op_sizes::byte,
        basecode::operand_encoding_t::flags::none,
        0);

    basecode::instruction_emitter fn_square_emitter(bootstrap_emitter.end_address());
    fn_square_emitter.load_stack_offset_to_register(
        basecode::op_sizes::dword,
        basecode::i_registers_t::i0,
        8);
    fn_square_emitter.multiply_int_register_to_register(
        basecode::op_sizes::dword,
        basecode::i_registers_t::i0,
        basecode::i_registers_t::i0,
        basecode::i_registers_t::i0);
    fn_square_emitter.store_register_to_stack_offset(
        basecode::op_sizes::dword,
        basecode::i_registers_t::i0,
        8);
    fn_square_emitter.rts();

    basecode::instruction_emitter main_emitter(fn_square_emitter.end_address());
    main_emitter.push_int_constant(
        basecode::op_sizes::dword,
        9);
    main_emitter.jump_subroutine_pc_relative(
        basecode::op_sizes::byte,
        basecode::operand_encoding_t::negative,
        main_emitter.end_address() - fn_square_emitter.start_address());
    main_emitter.pop_int_register(
        basecode::op_sizes::dword,
        basecode::i_registers_t::i5);
    main_emitter.push_int_constant(
        basecode::op_sizes::dword,
        5);
    main_emitter.jump_subroutine_pc_relative(
        basecode::op_sizes::byte,
        basecode::operand_encoding_t::negative,
        main_emitter.end_address() - fn_square_emitter.start_address());
    main_emitter.pop_int_register(
        basecode::op_sizes::dword,
        basecode::i_registers_t::i6);
    main_emitter.exit();

    bootstrap_emitter[0].patch_branch_address(
        main_emitter.start_address() - terp.program_start,
        1);
    bootstrap_emitter.encode(r, terp);
    fn_square_emitter.encode(r, terp);
    main_emitter.encode(r, terp);

    if (r.is_failed())
        return false;

    fmt::print("\nASSEMBLY LISTING:\n{}\n", terp.disassemble(r, terp.program_start));

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
    basecode::instruction_emitter bootstrap_emitter(terp.program_start);
    bootstrap_emitter.jump_pc_relative(
        basecode::op_sizes::byte,
        basecode::operand_encoding_t::flags::none,
        0);

    basecode::instruction_emitter fn_fibonacci(bootstrap_emitter.end_address());
    fn_fibonacci.load_stack_offset_to_register(
        basecode::op_sizes::dword,
        basecode::i_registers_t::i0,
        8);

    fn_fibonacci.push_int_register(
        basecode::op_sizes::dword,
        basecode::i_registers_t::i0);
    fn_fibonacci.trap(1);

    fn_fibonacci.compare_int_register_to_constant(
        basecode::op_sizes::dword,
        basecode::i_registers_t::i0,
        0);
    auto first_branch_address = fn_fibonacci.end_address();
    fn_fibonacci.branch_pc_relative_if_equal(
        basecode::op_sizes::byte,
        basecode::operand_encoding_t::none,
        0);

    fn_fibonacci.compare_int_register_to_constant(
        basecode::op_sizes::dword,
        basecode::i_registers_t::i0,
        1);
    auto second_branch_address = fn_fibonacci.end_address();
    fn_fibonacci.branch_pc_relative_if_equal(
        basecode::op_sizes::byte,
        basecode::operand_encoding_t::none,
        0);

    auto jump_over_rts_address = fn_fibonacci.end_address();
    fn_fibonacci.jump_pc_relative(
        basecode::op_sizes::byte,
        basecode::operand_encoding_t::none,
        0);

    auto label_exit_fib = fn_fibonacci.end_address();
    fn_fibonacci[4].patch_branch_address(label_exit_fib - first_branch_address, 1);
    fn_fibonacci[6].patch_branch_address(label_exit_fib - second_branch_address, 1);
    fn_fibonacci.rts();

    auto label_next_fib = fn_fibonacci.end_address();
    fn_fibonacci[7].patch_branch_address(label_next_fib - jump_over_rts_address, 1);

    fn_fibonacci.subtract_int_constant_from_register(
        basecode::op_sizes::dword,
        basecode::i_registers_t::i1,
        basecode::i_registers_t::i0,
        1);
    fn_fibonacci.subtract_int_constant_from_register(
        basecode::op_sizes::dword,
        basecode::i_registers_t::i2,
        basecode::i_registers_t::i0,
        2);
    fn_fibonacci.push_int_register(
        basecode::op_sizes::dword,
        basecode::i_registers_t::i2);
    fn_fibonacci.jump_subroutine_pc_relative(
        basecode::op_sizes::byte,
        basecode::operand_encoding_t::negative,
        fn_fibonacci.end_address() - fn_fibonacci.start_address());
    fn_fibonacci.pop_int_register(
        basecode::op_sizes::dword,
        basecode::i_registers_t::i2);
    fn_fibonacci.add_int_register_to_register(
        basecode::op_sizes::dword,
        basecode::i_registers_t::i1,
        basecode::i_registers_t::i1,
        basecode::i_registers_t::i2);
    fn_fibonacci.push_int_register(
        basecode::op_sizes::dword,
        basecode::i_registers_t::i1);
    fn_fibonacci.jump_subroutine_pc_relative(
        basecode::op_sizes::byte,
        basecode::operand_encoding_t::negative,
        fn_fibonacci.end_address() - fn_fibonacci.start_address());
    fn_fibonacci.pop_int_register(
        basecode::op_sizes::dword,
        basecode::i_registers_t::i1);
    fn_fibonacci.store_register_to_stack_offset(
        basecode::op_sizes::dword,
        basecode::i_registers_t::i1,
        8);
    fn_fibonacci.rts();

    basecode::instruction_emitter main_emitter(fn_fibonacci.end_address());
    main_emitter.push_int_constant(basecode::op_sizes::dword, 100);
    main_emitter.jump_subroutine_pc_relative(
        basecode::op_sizes::byte,
        basecode::operand_encoding_t::negative,
        main_emitter.end_address() - fn_fibonacci.start_address());
    main_emitter.dup();
    main_emitter.pop_int_register(
        basecode::op_sizes::dword,
        basecode::i_registers_t::i0);
    main_emitter.trap(1);
    main_emitter.exit();

    bootstrap_emitter[0].patch_branch_address(
        main_emitter.start_address() - terp.program_start,
        1);
    bootstrap_emitter.encode(r, terp);
    fn_fibonacci.encode(r, terp);
    main_emitter.encode(r, terp);

    fmt::print("\nASSEMBLY LISTING:\n{}\n", terp.disassemble(r, terp.program_start));
    auto result = run_terp(r, terp);
    //terp.dump_state(2);

    if (terp.register_file().i[0] != 1) {
        r.add_message("T001", "fn_fibonacci should end with 1 or 0.", true);
    }

    return result;
}

static int time_test_function(
        basecode::terp& terp,
        const std::string& title,
        const test_function_callable& test_function) {
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    basecode::result r;

    terp.reset();
    auto rc = test_function(r, terp);

    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    fmt::print("function: {} {}\n", title, rc ? "SUCCESS" : "FAILED");
    if (!rc || r.is_failed()) {
        print_results(r);
    }
    fmt::print("execution time (micro seconds): {}\n\n", duration);

    return rc;
}

static int terp_tests() {
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    basecode::terp terp(heap_size, stack_size);
    terp.register_trap(1, [](basecode::terp* terp) {
        auto value = terp->pop();
        fmt::print("[trap 1] ${:016X}\n", value);
    });

    basecode::result r;
    if (!terp.initialize(r)) {
        fmt::print("terp initialize failed.\n");
        print_results(r);
        return 1;
    }

    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    fmt::print("terp startup time (in microseconds): {}\n\n", duration);

    time_test_function(terp, "test_branches", test_branches);
    time_test_function(terp, "test_square", test_square);
    time_test_function(terp, "test_fibonacci", test_fibonacci);

    return 0;
}

static int compiler_tests() {
    basecode::compiler compiler(heap_size, stack_size);
    basecode::result r;
    if (!compiler.initialize(r)) {
        print_results(r);
        return 1;
    }

    //
    // basecode heap (as seen by the terp)
    //
    // +-----------------------------+ +--> top of heap (address: $2000000)
    // |                             | |
    // | stack (unbounded)           | | stack grows down, e.g. SUB SP, SP, 8 "allocates" 8 bytes on stack
    // |                             | *
    // +-----------------------------+
    // |                             |
    // |                             |
    // | free space                  | * --> how does alloc() and free() work in this space?
    // | arenas                      | |
    // |                             | +--------> context available to all functions
    // |                             |          | .allocator can be assigned callable
    // +-----------------------------+
    // |                             | * byte code top address
    // |                             |
    // |                             | *
    // //~~~~~~~~~~~~~~~~~~~~~~~~~~~// | generate stack sizing code for variables,
    // |                             | | function declarations, etc.
    // |                             | *
    // |                             |
    // |                             | * internal support functions
    // |                             | | some swi vectors will point at these
    // //~~~~~~~~~~~~~~~~~~~~~~~~~~~// |
    // |                             | | program code & data grows "up" in heap
    // |                             | * address: $90 start of free memory
    // |                             | |
    // |                             | |                   - reserved space
    // |                             | |                   - free space start address
    // |                             | |                   - stack top max address
    // |                             | |                   - program start address
    // |                             | | address: $80 -- start of heap metadata
    // |                             | | address: $0  -- start of swi vector table
    // +-----------------------------+ +--> bottom of heap (address: $0)
    //
    //
    // step 1. parse source to ast
    //
    // step 2. expand @import or @compile attributes by parsing to ast
    //
    // step 3. fold constants/type inference
    //
    // step 4. bytecode generation
    //
    // step 5. @run expansion
    //
    // short_string {
    //      capacity:u16 := 64;
    //      length:u16 := 0;
    //      data:*u8 := alloc(capacity);
    // } size_of(type(short_string)) == 12;
    //
    // string {
    //      capacity:u32 := 64;
    //      length:u32 := 0;
    //      data:*u8 := alloc(capacity);
    // } size_of(type(string)) == 64;
    //
    // array {
    //      capacity:u32 := 64;
    //      length:u32 := 0;
    //      element_type:type := type(any);
    //      data:*u8 := alloc(capacity * size_of(element));
    // } size_of(type(array)) == 72;
    //
    // callable_parameter {
    //      name:short_string;
    //      type:type;
    //      default:any := empty;
    //      spread:bool := false;
    // } size_of(type(callable_parameter)) == 32;
    //
    // callable {
    //      params:callable_parameter[];
    //      returns:callable_parameter[];
    //      address:*u8 := null;
    // } size_of(type(callable)) == 153;
    //
    std::stringstream source(
        "// this is a test comment\n"
        "// fibonacci sequence in basecode-alpha\n"
        "\n"
        "@entry_point main;\n"
        "\n"
//        "// string is a special case....\n"
//        "name:string := \"this is a test string literal\";\n"
//        "// pure copy of primitive structure fields\n"
//        "name_copy:string := name;\n"
//        "// move name into name_moved_copy\n"
//        "name_moved_copy:string := move(name);\n"
//        "// deep copy of string data\n"
//        "name_deep_copy:string := deep_copy(name);\n"
        "\n"
        "// this is a comment right before a variable assignment\n"
        "truth:bool := true;\n"
        "lies:bool := false;\n"
        "char:u8 := 'A';\n"
        "dx:u32 := 467;\n"
        "dx1:u32 := dx;\n"
        "vx:f64 := 3.145;\n"
        "vy:f64 := 1.112233;\n"
//        "name_ptr:*u8 := address_of(name);\n"
//        "name_ptr := null;\n"
        "\n"
        "foo:u16 := $ff * (($7f * 2) | %1000_0000_0000_0000);\n"
        "\n"
        // compile_time_thingy lives on the stack as aggregate callable
//        "compile_time_thingy := fn(a:u32, b:u32):u32 {\n"
        //  LOAD.DW     I0, SP, 12
        //  LOAD.DW     I1, SP, 8
        //  MUL.DW      I0, I0, I1
        //  STORE.DW    I0, SP, 16
        //  RTS
//        "   a * b;\n"
//        "};\n"
//        "\n"
        //
        //  SUB.QW      SP, SP, 12
        //  STORE.DW    6, SP, 4
        //  STORE.DW    6, SP, 8
        //  JSR         #$compile_time_thingy
        //  LOAD.DW     I0, SP, 16
        //  ADD.QW      SP, SP, 12
        //
//        "product_example:u32 := @run compile_time_thingy(6, 6);\n"
//        "\n"
//        "source:string := \"some_calc_value := fn():u16 { ($ff - $7f) * 2 == $fe; };\";\n"
//        "@import source;\n"
//        "\n"
//        "the_result_from_string:u16 := @run some_calc_value();\n"
        "\n"
        "fib := fn(n:u64):u64 {\n"
        "    if n == 0 || n == 1 {\n"
        "        return n;\n"
        "    } else {\n"
        "        return fib((n - 1) + fib(n - 2));\n"
        "    };\n"
        "};\n"
        "\n"
        "main := fn():u64 {\n"
        "    return fib(100);\n"
        "};");

    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    if (!compiler.compile(r, source)) {
        print_results(r);
        return 1;
    }

    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    fmt::print("compile time (in us): {}\n\n", duration);

    return 0;
}

int main() {
    int result = 0;

    result = compiler_tests();
    if (result != 0) return result;

    result = terp_tests();
    return result;
}