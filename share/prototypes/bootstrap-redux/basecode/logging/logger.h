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

#include <string>
#include <cstdint>
#include <string_view>
#include <basecode/format/format.h>

namespace basecode::logging {

    using namespace std::literals;

    enum class log_level_t {
        info,
        debug,
        trace,
        warn,
        error,
        critical
    };

    std::string_view log_level_name(log_level_t level);

    class logger_t {
    public:
        virtual ~logger_t() = default;

        template <typename... Args>
        void info(std::string_view format, const Args& ... args) {
            on_log(log_level_t::info, format::format(format, args...));
        }

        template <typename... Args>
        void warn(std::string_view format, const Args& ... args) {
            on_log(log_level_t::warn, format::format(format, args...));
        }

        template <typename... Args>
        void debug(std::string_view format, const Args& ... args) {
            on_log(log_level_t::debug, format::format(format, args...));
        }

        template <typename... Args>
        void trace(std::string_view format, const Args& ... args) {
            on_log(log_level_t::trace, format::format(format, args...));
        }

        template <typename... Args>
        void error(std::string_view format, const Args& ... args) {
            on_log(log_level_t::error, format::format(format, args...));
        }

        template <typename... Args>
        void critical(std::string_view format, const Args& ... args) {
            on_log(log_level_t::critical, format::format(format, args...));
        }

    protected:
        virtual void on_log(log_level_t level, std::string_view message) = 0;
    };

}