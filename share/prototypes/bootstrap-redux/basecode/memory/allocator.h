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

#include <cstdint>
#include <optional>

namespace basecode::memory {

    struct header_t final {
        uint32_t size;
    };

    const uint32_t header_pad_value = 0xffffffffu;

    inline header_t* header(void* data) {
        auto p = static_cast<uint32_t*>(data);
        while (p[-1] == header_pad_value)
            --p;
        return reinterpret_cast<header_t*>(p - 1);
    }

    inline void* align_forward(void* p, uint32_t align) {
        auto pi = uintptr_t(p);
        const uint32_t mod = pi % align;
        if (mod)
            pi += (align - mod);
        return (void*) pi;
    }

    inline void* data_pointer(header_t* header, uint32_t align) {
        void* p = header + 1;
        return align_forward(p, align);
    }

    inline void fill(header_t* header, void* data, uint32_t size) {
        header->size = size;
        auto p = reinterpret_cast<uint32_t*>(header + 1);
        while (p < data)
            *p++ = header_pad_value;
    }

    inline uint32_t size_with_padding(uint32_t size, uint32_t align) {
        return size + align + sizeof(header_t);
    }

    ///////////////////////////////////////////////////////////////////////////

    class allocator_t {
    public:
        static const uint32_t default_align = 4;

        allocator_t() = default;

        allocator_t(const allocator_t& other) = delete;

        allocator_t& operator=(const allocator_t& other) = delete;

        virtual ~allocator_t() = default;

        virtual void* allocate(
            uint32_t size,
            uint32_t align = default_align) = 0;

        virtual void deallocate(void* p) = 0;

        virtual std::optional<uint32_t> total_allocated() = 0;

        virtual std::optional<uint32_t> allocated_size(void* p) = 0;
    };

}