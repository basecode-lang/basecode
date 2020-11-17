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

#include "murmur.h"

namespace basecode::hashing::murmur {

    uint32_t hash32(const void* src, size_t len) {
        return hash32(src, len, 0x9747b28c);
    }

    uint32_t hash32(const void* src, size_t len, uint32_t seed) {
        uint32_t const c1 = 0xcc9e2d51;
        uint32_t const c2 = 0x1b873593;
        uint32_t const r1 = 15;
        uint32_t const r2 = 13;
        uint32_t const m = 5;
        uint32_t const n = 0xe6546b64;

        int32_t i, nblocks = len / 4;
        uint32_t hash = seed, k1 = 0;
        auto const* blocks = static_cast<uint32_t const*>(src);
        uint8_t const* tail = static_cast<uint8_t const*>(src) + nblocks * 4;

        for (i = 0; i < nblocks; i++) {
            uint32_t k = blocks[i];
            k *= c1;
            k = (k << r1) | (k >> (32 - r1));
            k *= c2;

            hash ^= k;
            hash = ((hash << r2) | (hash >> (32 - r2))) * m + n;
        }

        switch (len & 3) {
            case 3:
                k1 ^= tail[2] << 16;
            case 2:
                k1 ^= tail[1] << 8;
            case 1:
                k1 ^= tail[0];

                k1 *= c1;
                k1 = (k1 << r1) | (k1 >> (32 - r1));
                k1 *= c2;
                hash ^= k1;
        }

        hash ^= len;
        hash ^= (hash >> 16);
        hash *= 0x85ebca6b;
        hash ^= (hash >> 13);
        hash *= 0xc2b2ae35;
        hash ^= (hash >> 16);

        return hash;
    }

    uint64_t hash64(const void* src, size_t len) {
        return hash64(src, len, 0x9747b28c);
    }

    uint64_t hash64(const void* src, size_t len, uint64_t seed) {
        uint64_t const m = 0xc6a4a7935bd1e995ULL;
        int32_t const r = 47;

        uint64_t h = seed ^(len * m);

        auto const* data = static_cast<uint64_t const*>(src);
        auto const* data2 = static_cast<uint8_t const*>(src);
        uint64_t const* end = data + (len / 8);

        while (data != end) {
            uint64_t k = *data++;

            k *= m;
            k ^= k >> r;
            k *= m;

            h ^= k;
            h *= m;
        }

        switch (len & 7) {
            case 7:
                h ^= static_cast<uint64_t>(data2[6]) << 48;
            case 6:
                h ^= static_cast<uint64_t>(data2[5]) << 40;
            case 5:
                h ^= static_cast<uint64_t>(data2[4]) << 32;
            case 4:
                h ^= static_cast<uint64_t>(data2[3]) << 24;
            case 3:
                h ^= static_cast<uint64_t>(data2[2]) << 16;
            case 2:
                h ^= static_cast<uint64_t>(data2[1]) << 8;
            case 1:
                h ^= static_cast<uint64_t>(data2[0]);
                h *= m;
        }

        h ^= h >> r;
        h *= m;
        h ^= h >> r;

        return h;
    }
}