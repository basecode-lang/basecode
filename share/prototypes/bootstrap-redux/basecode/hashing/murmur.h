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
#include <cstddef>

namespace basecode::hashing::murmur {

    uint32_t hash32(const void* src, size_t len);

    uint32_t hash32(const void* src, size_t len, uint32_t seed);

    uint64_t hash64(const void* src, size_t len);

    uint64_t hash64(const void* src, size_t len, uint64_t seed);

}
