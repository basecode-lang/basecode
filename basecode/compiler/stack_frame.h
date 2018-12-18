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

#include "compiler_types.h"
#include "stack_frame_entry.h"

namespace basecode::compiler {

    // +-------------+
    // | ...         |
    // | locals      | -offsets
    // | fp          | 0
    // | return addr | +offsets
    // | return slot |
    // | param 1     |
    // | param 2     |
    // +-------------+
    //
    class stack_frame {
    public:
        explicit stack_frame(stack_frame* parent_frame);

        stack_frame_entry* add(
            stack_frame_entry_type_t type,
            const std::string& name,
            size_t size_in_bytes);

        stack_frame* parent_frame();

        stack_frame_base_offsets_t& offsets();

        stack_frame_entry* find(const std::string& name);

        size_t type_size_in_bytes(stack_frame_entry_type_t type) const;

    private:
        stack_frame* _parent_frame = nullptr;
        stack_frame_base_offsets_t _offsets {};
        std::unordered_map<std::string, stack_frame_entry> _entries {};
        std::unordered_map<stack_frame_entry_type_t, size_t> _type_sizes {};
    };

};

