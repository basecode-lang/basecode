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

#include "spd_logger.h"

namespace basecode::logging {

    spd_logger_t::spd_logger_t(spdlog::logger* logger): _logger(logger) {
        assert(logger);
    }

    bool spd_logger_t::initialize(
            result_t& r,
            spdlog::level::level_enum level) {
        _logger->set_level(level);
        _logger->set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
        return true;
    }

    bool spd_logger_t::shutdown(result_t& r) {
        memory::destroy(_logger);
        return true;
    }

    void spd_logger_t::on_log(log_level_t level, std::string_view message) {
        switch (level) {
            case log_level_t::info:
                _logger->info(message);
                break;
            case log_level_t::debug:
                _logger->debug(message);
                break;
            case log_level_t::trace:
                _logger->trace(message);
                break;
            case log_level_t::warn:
                _logger->warn(message);
                break;
            case log_level_t::error:
                _logger->error(message);
                break;
            case log_level_t::critical:
                _logger->critical(message);
                break;
        }
    }

}
