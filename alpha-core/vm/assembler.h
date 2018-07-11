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
#include "segment.h"

namespace basecode::vm {

    class terp;
    class instruction_block;

    class assembler {
    public:
        explicit assembler(vm::terp* terp);

        virtual ~assembler();

        bool assemble(
            common::result& r,
            vm::instruction_block* block);

        vm::segment* segment(
            const std::string& name,
            segment_type_t type);

        bool assemble_from_source(
            common::result& r,
            std::istream& source);

        instruction_block* pop_block();

        instruction_block* root_block();

        bool in_procedure_scope() const;

        segment_list_t segments() const;

        instruction_block* current_block();

        void push_block(instruction_block* block);

        vm::segment* segment(const std::string& name);

        instruction_block* make_basic_block(instruction_block* parent_block = nullptr);

        instruction_block* make_procedure_block(instruction_block* parent_block = nullptr);

    private:
        void add_new_block(instruction_block* block);

    private:
        vm::terp* _terp = nullptr;
        uint64_t _location_counter = 0;
        uint32_t _procedure_block_count = 0;
        std::vector<instruction_block*> _blocks {};
        std::stack<instruction_block*> _block_stack {};
        std::unordered_map<std::string, vm::segment> _segments {};
    };

};

