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

#include <mutex>
#include <thread>
#include <chrono>
#include <condition_variable>

namespace basecode::threading {

    class manual_reset_event_t final {
    public:
        explicit manual_reset_event_t(bool initial = false);

        manual_reset_event_t(const manual_reset_event_t&) = delete;

        manual_reset_event_t& operator=(const manual_reset_event_t&) = delete;

        void set();

        void reset();

        bool wait_one();

        bool wait_one(std::chrono::milliseconds timeout);

    private:
        bool _flag;
        std::mutex _protect;
        std::condition_variable _signal;
    };

}