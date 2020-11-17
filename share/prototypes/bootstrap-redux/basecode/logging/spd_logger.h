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

#include <memory>
#include <spdlog/logger.h>
#include <basecode/types.h>
#include <basecode/result.h>
#include "logger.h"

namespace basecode::logging {

    class spd_logger_t : public logger_t {
    public:
        explicit spd_logger_t(spdlog::logger* logger);

        bool initialize(
            result_t& r,
            spdlog::level::level_enum level = spdlog::level::level_enum::trace);

        bool shutdown(result_t& r);

    protected:
        void on_log(log_level_t level, std::string_view message) override;

    private:
        spdlog::logger* _logger{};
    };

}