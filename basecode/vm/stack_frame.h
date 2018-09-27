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

#include <map>
#include <stack>
#include <string>
#include <cstdint>

namespace basecode::vm {

    enum class stack_frame_entry_type_t : uint8_t {
        local = 1,
        parameter,
        return_slot
    };

    inline static std::string stack_frame_entry_type_name(stack_frame_entry_type_t type) {
        switch (type) {
            case stack_frame_entry_type_t::local:
                return "local";
            case stack_frame_entry_type_t::parameter:
                return "parameter";
            case stack_frame_entry_type_t::return_slot:
                return "return_slot";
        }
        return "unknown";
    }

    struct stack_frame_entry_t {
        int32_t offset;
        std::string name;
        stack_frame_entry_type_t type;
    };

    struct stack_frame_t;

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

