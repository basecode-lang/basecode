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

#include <thread>
#include <cpuid.h>
#include <basecode/errors/errors.h>
#include "timer.h"

namespace basecode::profiler {

    static double s_clock_cycles_per_ns;

    double calibrate() {
        auto first = rtdscp();
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        auto second = rtdscp();
        return (second - first) * 4.0e-9;
    }

    void cpuid(uint32_t* regs, uint32_t leaf) {
        __get_cpuid(leaf, regs, regs + 1, regs + 2, regs + 3);
    }

    ///////////////////////////////////////////////////////////////////////////

    bool shutdown(result_t& r) {
        return true;
    }

    bool initialize(result_t& r) {
        uint32_t regs[4];

        cpuid(regs, 0x80000001);
        if (!(regs[3] & (1 << 27))) {
            errors::add_error(r, errors::profiler::no_cpu_rtdscp_support);
            return false;
        }

        cpuid(regs, 0x80000007);
        if (!(regs[3] & (1 << 8))) {
            errors::add_error(r, errors::profiler::no_cpu_invariant_tsc_support);
            return false;
        }

        s_clock_cycles_per_ns = calibrate();

        return true;
    }

    ///////////////////////////////////////////////////////////////////////////

    void timer_t::stop() {
        _end = rtdscp();
    }

    void timer_t::start() {
        _start = rtdscp();
    }

    uint64_t timer_t::elapsed() const {
        double delta = _end - _start;
        return delta / s_clock_cycles_per_ns;
    }

}