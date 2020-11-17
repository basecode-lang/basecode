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
#include <basecode/types.h>
#include <basecode/result.h>

namespace basecode::profiler {

    double calibrate();

    factor_force_inline uint64_t rtdsc() {
        uint32_t lo, hi;
        __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
        return (uint64_t) hi << 32 | lo;
    }

    factor_force_inline uint64_t rtdscp() {
        uint32_t eax, edx;
        asm volatile ( "rdtscp" : "=a" (eax), "=d" (edx)::"%ecx" );
        return (uint64_t(edx) << 32) + uint64_t(eax);
    }

    void cpuid(uint32_t* regs, uint32_t leaf);

    ///////////////////////////////////////////////////////////////////////////

    bool shutdown(result_t& r);

    bool initialize(result_t& r);

    ///////////////////////////////////////////////////////////////////////////

    class timer_t final {
    public:
        timer_t() = default;

        void stop();

        void start();

        [[nodiscard]] uint64_t elapsed() const;

    private:
        uint64_t _end;
        uint64_t _start;
    };

}