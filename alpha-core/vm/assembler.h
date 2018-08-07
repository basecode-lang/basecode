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

#include <stack>
#include <vector>
#include <unordered_map>
#include <common/result.h>
#include <common/id_pool.h>
#include "terp.h"
#include "segment.h"
#include "assembly_listing.h"
#include "register_allocator.h"

namespace basecode::vm {

    enum class target_register_type_t {
        none,
        integer,
        floating_point
    };

    struct target_register_t {
        op_sizes size;
        target_register_type_t type;
        union {
            i_registers_t i;
            f_registers_t f;
        } reg;
    };

    class instruction_block;

    class assembler {
    public:
        explicit assembler(vm::terp* terp);

        virtual ~assembler();

        bool assemble(
            common::result& r,
            vm::instruction_block* block = nullptr);

        vm::segment* segment(
            const std::string& name,
            segment_type_t type);

        bool assemble_from_source(
            common::result& r,
            std::istream& source);

        instruction_block* pop_block();

        instruction_block* root_block();

        vm::assembly_listing& listing();

        bool in_procedure_scope() const;

        segment_list_t segments() const;

        void free_reg(i_registers_t reg);

        void free_reg(f_registers_t reg);

        bool initialize(common::result& r);

        instruction_block* current_block();

        bool allocate_reg(i_registers_t& reg);

        bool allocate_reg(f_registers_t& reg);

        bool resolve_labels(common::result& r);

        bool apply_addresses(common::result& r);

        void push_block(instruction_block* block);

        target_register_t pop_target_register();

        target_register_t* current_target_register();

        vm::segment* segment(const std::string& name);

        void push_target_register(op_sizes size, i_registers_t reg);

        void push_target_register(op_sizes size, f_registers_t reg);

        instruction_block* make_basic_block(instruction_block* parent_block = nullptr);

        instruction_block* make_procedure_block(instruction_block* parent_block = nullptr);

    private:
        void add_new_block(instruction_block* block);

    private:
        vm::terp* _terp = nullptr;
        uint64_t _location_counter = 0;
        vm::assembly_listing _listing {};
        uint32_t _procedure_block_count = 0;
        std::vector<instruction_block*> _blocks {};
        std::stack<instruction_block*> _block_stack {};
        std::stack<target_register_t> _target_registers {};
        std::unordered_map<std::string, vm::segment> _segments {};
        register_allocator_t<i_registers_t> _i_register_allocator {};
        register_allocator_t<f_registers_t> _f_register_allocator {};
    };

};

