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

#include <set>
#include <stack>
#include <cstdint>
#include "terp.h"

namespace basecode::vm {

    struct register_allocator_t {
        register_allocator_t() {
            reset();
        }

        void reset() {
            used.clear();
            while (!available.empty())
                available.pop();
            for (int8_t r = 63; r >= 0; r--)
                available.push(static_cast<registers_t>(r));
        }

        void free(registers_t reg) {
            if (used.erase(reg) > 0) {
                available.push(reg);
            }
        }

        bool allocate(registers_t& reg) {
            if (available.empty())
                return false;
            reg = available.top();
            available.pop();
            used.insert(reg);
            return true;
        }

        std::set<registers_t> used {};
        std::stack<registers_t> available {};
    };

};

