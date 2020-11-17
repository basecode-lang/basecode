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

#include "logger.h"

namespace basecode::logging {

    std::string_view log_level_name(log_level_t level) {
        switch (level) {
            case log_level_t::info:     return "INFO"sv;
            case log_level_t::debug:    return "DEBUG"sv;
            case log_level_t::trace:    return "TRACE"sv;
            case log_level_t::warn:     return "WARN"sv;
            case log_level_t::error:    return "ERROR"sv;
            case log_level_t::critical: return "CRITICAL"sv;
        }
    }

}