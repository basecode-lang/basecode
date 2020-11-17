// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
//       C O M P I L E R  P R O J E C T
//
// Copyright (C) 2019 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <string_view>
#include <basecode/context/context.h>
#include "allocator.h"

namespace basecode::memory {

    class scratch_allocator_t : public allocator_t {
    public:
        explicit scratch_allocator_t(
            uint32_t size,
            allocator_t* backing = context::current()->allocator);

        ~scratch_allocator_t() override;

        bool in_use(void *p);

        void* allocate(
            uint32_t size,
            uint32_t align) override;

        void deallocate(void* p) override;

        std::optional<uint32_t> total_allocated() override;

        std::optional<uint32_t> allocated_size(void *p) override;

    private:
        uint8_t* _end{};
        uint8_t* _free{};
        uint8_t* _begin{};
        uint8_t* _allocate{};
        uint32_t _total_allocated{};
        allocator_t* _backing = nullptr;
    };

}