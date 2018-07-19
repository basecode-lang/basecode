// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <string>
#include <cstdint>
#include <vm/terp.h>
#include <filesystem>
#include "session.h"

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

namespace basecode::compiler {

    struct bytecode_emitter_options_t {
        size_t heap_size = 0;
        size_t stack_size = 0;
    };

    class bytecode_emitter {
    public:
        explicit bytecode_emitter(const bytecode_emitter_options_t& options);

        virtual ~bytecode_emitter();

        bool compile(
            common::result& r,
            compiler::session& session);

        bool initialize(common::result& r);

    private:
        vm::terp _terp;
        bytecode_emitter_options_t _options {};
    };

};