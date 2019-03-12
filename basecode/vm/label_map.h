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
#include <unordered_map>
#include "label.h"

namespace basecode::vm {

    class label_map {
    public:
        label_map() = default;

        vm::label* make(
            const std::string& name,
            basic_block* block);

        const vm::label* find(const std::string& name) const;

        void add_cfg_edge(vm::basic_block* from, const std::string& to);

    private:
        void apply_cfg_edge(vm::basic_block* from, vm::basic_block* to);

    private:
        std::unordered_map<std::string, vm::label> _labels {};
        std::unordered_map<std::string, basic_block_stack_t> _edges {};
    };

}

