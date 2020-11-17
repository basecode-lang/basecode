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

#include <basecode/memory/system.h>
#include "pool.h"

namespace basecode::strings {

    pool_t::pool_t(memory::allocator_t* allocator) : _allocator(allocator),
                                                     _index(context::current()->allocator) {
    }

    std::string_view pool_t::intern(std::string_view value) {
        if (value.empty())
            return value;

        auto data = _index.find(value);
        if (!data) {
            auto data_ptr = _allocator->allocate(value.length());
            std::memcpy(data_ptr, value.data(), value.length());
            _index.insert(value, data_ptr);
            return std::string_view(static_cast<char*>(data_ptr), value.length());
        }
        return std::string_view(static_cast<char*>(data), value.length());
    }

}
