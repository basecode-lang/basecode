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

#include "instruction_block.h"

namespace basecode::vm {

    instruction_block::instruction_block() {
    }

    instruction_block::~instruction_block() {
        clear_blocks();
        clear_labels();
        clear_instructions();
    }

    void instruction_block::clear_labels() {
        for (const auto& it : _labels)
            delete it.second;
        _labels.clear();
        _label_to_instruction_map.clear();
    }

    void instruction_block::clear_blocks() {
        _blocks.clear();
    }

    void instruction_block::push(double value) {
    }

    void instruction_block::push(uint64_t value) {
    }

    void instruction_block::clear_instructions() {
        _instructions.clear();
    }

    void instruction_block::call(const std::string& proc_name) {
    }

    void instruction_block::add_block(instruction_block* block) {
        _blocks.push_back(block);
    }

    void instruction_block::remove_block(instruction_block* block) {
        auto it = std::find_if(
            _blocks.begin(),
            _blocks.end(),
            [&block](auto each) { return each == block; });
        if (it == _blocks.end())
            return;
        _blocks.erase(it);
    }

    vm::label* instruction_block::make_label(const std::string& name) {
        auto label = new vm::label(name);
        _labels.insert(std::make_pair(name, label));
        _label_to_instruction_map.insert(std::make_pair(name, _instructions.size()));
        return label;
    }

};