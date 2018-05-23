// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <string>
#include <cstdint>
#include <vm/terp.h>
#include <filesystem>
#include <parser/parser.h>
#include "scope.h"

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

namespace basecode::compiler {

    struct bytecode_emitter_options_t {
        bool verbose = false;
        size_t heap_size = 0;
        size_t stack_size = 0;
        std::filesystem::path ast_graph_file_name {};
    };

    class bytecode_emitter {
    public:
        explicit bytecode_emitter(const bytecode_emitter_options_t& options);

        virtual ~bytecode_emitter();

        bool compile_files(
            common::result& r,
            const std::vector<std::filesystem::path>& source_files);

        bool initialize(common::result& r);

        bool compile(common::result& r, std::istream& input);

        bool compile_stream(common::result& r, std::istream& input);

    private:
        void build_scope_tree(
            common::result& r,
            compiler::scope* scope,
            const syntax::ast_node_shared_ptr& node);

        void apply_constant_folding(
            common::result& r,
            const syntax::ast_node_shared_ptr& node);

    private:
        vm::terp _terp;
        scope _global_scope;
        bytecode_emitter_options_t _options {};
    };

};