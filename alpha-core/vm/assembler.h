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

        segment_list_t segments() const;

        instruction_block* current_block();

        instruction_block* make_new_block();

        void push_block(instruction_block* block);

        vm::segment* segment(const std::string& name);

    private:
        vm::terp* _terp = nullptr;
        uint64_t _location_counter = 0;
        std::vector<instruction_block*> _blocks {};
        std::stack<instruction_block*> _block_stack {};
        std::unordered_map<std::string, vm::segment> _segments {};
    };

};

