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
#include "allocator.h"

namespace basecode::memory {

    class proxy_allocator_t : public allocator_t {
    public:
        proxy_allocator_t(adt::string_t name, allocator_t* backing);

        ~proxy_allocator_t() override;

        void* allocate(
            uint32_t size,
            uint32_t align) override;

        void deallocate(void* p) override;

        [[nodiscard]] std::string_view name() const;

        std::optional<uint32_t> total_allocated() override;

        std::optional<uint32_t> allocated_size(void *p) override;

    private:
        adt::string_t _name;
        allocator_t* _backing{};
    };

}