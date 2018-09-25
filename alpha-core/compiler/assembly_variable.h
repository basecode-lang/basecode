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
#include <fmt/format.h>
#include <vm/assembler.h>
#include <common/defer.h>
#include "session.h"
#include "compiler_types.h"

namespace basecode::compiler {

    struct variable_register_t {
        bool matches(vm::register_t* other_reg);

        bool reserve(compiler::session& session);

        void release(compiler::session& session);

        bool allocated = false;
        vm::register_t reg;
    };

    ///////////////////////////////////////////////////////////////////////////

    struct variable_t {
        bool init(compiler::session& session);

        bool read(compiler::session& session);

        bool write(compiler::session& session);

        void make_live(compiler::session& session);

        void make_dormant(compiler::session& session);

        std::string name;
        bool live = false;
        bool written = false;
        identifier_usage_t usage;
        int64_t address_offset = 0;
        bool requires_read = false;
        bool address_loaded = false;
        variable_register_t value_reg;
        compiler::type* type = nullptr;
        variable_register_t address_reg {
            .reg = {
                .size = vm::op_sizes::qword,
                .type = vm::register_type_t::integer
            },
        };
        vm::stack_frame_entry_t* frame_entry = nullptr;
    };

    ///////////////////////////////////////////////////////////////////////////

    class assembly_variable {
    public:

    private:
    };

};

