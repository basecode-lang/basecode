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

#include "hashable.h"

namespace basecode::adt {

    template <> uint64_t hash_key(const string_t& key) {
        return hashing::murmur::hash64(key.begin(), key.size());
    }

    template <> uint64_t hash_key(const std::string_view& key) {
        return hashing::murmur::hash64(key.data(), key.length());
    }

}