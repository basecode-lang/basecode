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

#include "terp.h"
#include "assembler.h"
#include "instruction_block.h"
#include "instruction_emitter.h"

namespace basecode::vm {

    assembler::assembler(vm::terp* terp) : _terp(terp) {
        _location_counter = _terp->heap_vector(heap_vectors_t::program_start);
    }

    assembler::~assembler() {
        for (auto block : _blocks)
            delete block;
        _blocks.clear();
    }

    bool assembler::assemble(
            common::result& r,
            vm::instruction_block* block) {
        // write out bootstrap jmp

        // write out data segments

        // write out instruction blocks

        return false;
    }

    vm::segment* assembler::segment(
            const std::string& name,
            segment_type_t type) {
        _segments.insert(std::make_pair(
            name,
            vm::segment(name, type)));
        return segment(name);
    }

    bool assembler::assemble_from_source(
            common::result& r,
            std::istream& source) {
        return false;
    }

    instruction_block* assembler::pop_block() {
        if (_block_stack.empty())
            return nullptr;
        auto top = _block_stack.top();
        _block_stack.pop();
        return top;
    }

    segment_list_t assembler::segments() const {
        segment_list_t list {};
        for (const auto& it : _segments) {
            list.push_back(const_cast<vm::segment*>(&it.second));
        }
        return list;
    }

    instruction_block* assembler::current_block() {
        if (_block_stack.empty())
            return nullptr;
        return _block_stack.top();
    }

    instruction_block* assembler::make_new_block() {
        auto block = new instruction_block();
        _blocks.push_back(block);
        auto top_block = current_block();
        if (top_block != nullptr)
            top_block->add_block(block);
        return block;
    }

    void assembler::push_block(instruction_block* block) {
        _block_stack.push(block);
    }

    vm::segment* assembler::segment(const std::string& name) {
        auto it = _segments.find(name);
        if (it == _segments.end())
            return nullptr;
        return &it->second;
    }

};