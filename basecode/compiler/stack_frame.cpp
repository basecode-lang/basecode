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

#include <common/bytes.h>
#include <vm/instruction_block.h>
#include "stack_frame.h"

namespace basecode::compiler {

    stack_frame::stack_frame(
            stack_frame* parent_frame) : _parent_frame(parent_frame) {
    }

    stack_frame_entry* stack_frame::add(
            stack_frame_entry_type_t type,
            const std::string& name,
            size_t size_in_bytes) {
        int32_t offset = 0;
        auto size_it = _type_sizes.find(type);
        if (size_it == _type_sizes.end()) {
            _type_sizes.insert(std::make_pair(type, size_in_bytes));
        } else {
            offset = size_it->second;
            _type_sizes[type] = size_it->second + size_in_bytes;
        }
        auto it = _entries.insert(std::make_pair(
            name,
            stack_frame_entry(this, name, offset, type)));
        return &it.first->second;
    }

    stack_frame* stack_frame::parent_frame() {
        return _parent_frame;
    }

    stack_frame_base_offsets_t& stack_frame::offsets() {
        return _offsets;
    }

    stack_frame_entry* stack_frame::find(const std::string& name) {
        auto current_frame = this;
        while (current_frame != nullptr) {
            auto it = _entries.find(name);
            if (it != _entries.end())
                return &it->second;
            current_frame = current_frame->_parent_frame;
        }
        return nullptr;
    }

    size_t stack_frame::type_size_in_bytes(stack_frame_entry_type_t type) const {
        auto it = _type_sizes.find(type);
        if (it != _type_sizes.end()) {
            return common::align(it->second, 8);
        }
        return 0;
    }

};