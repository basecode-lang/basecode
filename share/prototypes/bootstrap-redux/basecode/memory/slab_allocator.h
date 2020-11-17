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

#include <basecode/adt/string.h>
#include <basecode/context/context.h>
#include "allocator.h"

namespace basecode::memory {

    class slab_allocator_t : public allocator_t {
        struct slab_t final {
            void* page{};
            slab_t* prev{};
            slab_t* next{};
            void* free_list{};
            uint32_t buffer_count{};
        };

    public:
        slab_allocator_t(
            adt::string_t name,
            uint32_t size,
            uint32_t align = default_align,
            allocator_t* backing = context::current()->allocator);

        ~slab_allocator_t() override;

        void* allocate(
            uint32_t size = 0,
            uint32_t align = default_align) override;

        void deallocate(void* p) override;

        [[nodiscard]] std::string_view name() const;

        std::optional<uint32_t> total_allocated() override;

        std::optional<uint32_t> allocated_size(void*) override;

    private:
        void grow();

        void remove(slab_t* slab);

        void move_back(slab_t* slab);

        void move_front(slab_t* slab);

    private:
        slab_t* _back{};
        slab_t* _front{};
        adt::string_t _name;
        allocator_t* _backing;
        uint32_t _buffer_size;
        uint32_t _buffer_align;
        uint32_t _page_count{};
        uint32_t _maximum_buffers{};
        uint32_t _total_allocated{};
    };

}
