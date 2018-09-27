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

#include "vm_types.h"

namespace basecode::vm {

    struct stack_frame_t {
        stack_frame_t(stack_frame_t* parent_frame);

        stack_frame_entry_t* add(
            stack_frame_entry_type_t type,
            const std::string& name,
            int32_t offset);

        stack_frame_entry_t* find(const std::string& name);

        stack_frame_entry_t* find_up(const std::string& name);

        stack_frame_t* parent_frame = nullptr;
        std::map<std::string, stack_frame_entry_t> entries {};
    };

};

