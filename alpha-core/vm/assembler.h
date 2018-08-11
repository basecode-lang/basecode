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

        void pop_target_register();

        instruction_block* pop_block();

        instruction_block* root_block();

        vm::assembly_listing& listing();

        bool in_procedure_scope() const;

        segment_list_t segments() const;

        bool initialize(common::result& r);

        instruction_block* current_block();

        bool allocate_reg(register_t& reg);

        void free_reg(const register_t& reg);

        register_t* current_target_register();

        bool resolve_labels(common::result& r);

        bool apply_addresses(common::result& r);

        void push_block(instruction_block* block);

        vm::segment* segment(const std::string& name);

        void push_target_register(const register_t& reg);

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
        register_allocator_t _register_allocator {};
        std::stack<register_t> _target_registers {};
        std::stack<instruction_block*> _block_stack {};
        std::unordered_map<std::string, vm::segment> _segments {};
    };

};

