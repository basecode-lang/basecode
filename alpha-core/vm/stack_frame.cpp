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

#include "stack_frame.h"
#include "instruction_block.h"

namespace basecode::vm {

    stack_frame_t::stack_frame_t(stack_frame_t* parent_frame) : parent_frame(parent_frame) {
    }

    stack_frame_entry_t* stack_frame_t::add(
            stack_frame_entry_type_t type,
            const std::string& name,
            int32_t offset) {
        auto it = entries.insert(std::make_pair(
            name,
            stack_frame_entry_t {
                .offset = offset,
                .name = name,
                .type = type
            }));
        return &it.first->second;
    }

    stack_frame_entry_t* stack_frame_t::find(const std::string& name) {
        auto it = entries.find(name);
        if (it == entries.end())
            return nullptr;
        return &it->second;
    }

    stack_frame_entry_t* stack_frame_t::find_up(const std::string& name) {
        auto current_frame = this;
        while (current_frame != nullptr) {
            auto entry = current_frame->find(name);
            if (entry != nullptr)
                return entry;
            if (current_frame->parent_frame == nullptr)
                break;
            current_frame = current_frame->parent_frame;
        }
        return nullptr;
    }

};