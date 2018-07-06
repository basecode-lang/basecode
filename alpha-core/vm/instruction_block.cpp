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
        for (const auto& it : _labels)
            delete it.second;
        _labels.clear();
    }

    void instruction_block::push(double value) {
    }

    void instruction_block::push(uint64_t value) {

    }

    void instruction_block::call(const std::string& proc_name) {

    }

    vm::label* instruction_block::make_label(const std::string& name) {
        auto label = new vm::label(name);
        _labels.insert(std::make_pair(name, label));
        _label_to_instruction_map.insert(std::make_pair(name, _instructions.size()));
        return label;
    }

};