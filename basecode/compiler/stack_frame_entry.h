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

namespace basecode::compiler {

    class stack_frame;

    class stack_frame_entry {
    public:
        stack_frame_entry(
            stack_frame* owning_frame,
            const std::string& name,
            int32_t offset,
            stack_frame_entry_type_t type);

        int32_t offset() const;

        std::string name() const;

        stack_frame* owning_frame() const;

        stack_frame_entry_type_t type() const;

    private:
        int32_t _offset = 0;
        std::string _name {};
        stack_frame* _owning_frame = nullptr;
        stack_frame_entry_type_t _type = stack_frame_entry_type_t::local;
    };

};

