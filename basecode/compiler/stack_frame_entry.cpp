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
#include "stack_frame_entry.h"

namespace basecode::compiler {

    stack_frame_entry::stack_frame_entry(
            stack_frame* owning_frame,
            const std::string& name,
            int32_t offset,
            stack_frame_entry_type_t type) : _offset(offset),
                                             _name(name),
                                             _owning_frame(owning_frame),
                                             _type(type) {
    }

    int32_t stack_frame_entry::offset() const {
        switch (_type) {
            case stack_frame_entry_type_t::local:
                return -_offset;
            case stack_frame_entry_type_t::parameter:
            case stack_frame_entry_type_t::return_slot:
                return _offset;
            default:
                break;
        }
        return 0;
    }

    std::string stack_frame_entry::name() const {
        return _name;
    }

    stack_frame* stack_frame_entry::owning_frame() const {
        return _owning_frame;
    }

    stack_frame_entry_type_t stack_frame_entry::type() const {
        return _type;
    }

};