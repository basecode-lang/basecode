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

#include "label_map.h"
#include "basic_block.h"

namespace basecode::vm {

    vm::label* label_map::make(
            const std::string& name,
            basic_block* block) {
        auto lit = _labels.insert(std::make_pair(
            name,
            vm::label(name, block)));

        auto label = &lit.first->second;

        auto eit = _edges.find(name);
        if (eit != std::end(_edges)) {
            auto& stack = eit->second;
            while (!stack.empty()) {
                apply_cfg_edge(stack.top(), label->block());
                stack.pop();
            }
            _edges.erase(name);
        }

        return label;
    }

    const vm::label* label_map::find(const std::string& name) const {
        const auto it = _labels.find(name);
        if (it == _labels.end())
            return nullptr;
        return &it->second;
    }

    void label_map::apply_cfg_edge(vm::basic_block* from, vm::basic_block* to) {
        from->successors().emplace_back(to);
        to->predecessors().emplace_back(from);
    }

    void label_map::add_cfg_edge(vm::basic_block* from, const std::string& to) {
        auto label = const_cast<vm::label*>(find(to));
        if (label != nullptr) {
            apply_cfg_edge(from, label->block());
            return;
        }

        basic_block_stack_t* stack = nullptr;
        auto it = _edges.find(to);
        if (it == std::end(_edges)) {
            auto result = _edges.insert(std::make_pair(to, basic_block_stack_t {}));
            stack = &result.first->second;
        } else {
            stack = &it->second;
        }

        stack->push(from);
    }

}