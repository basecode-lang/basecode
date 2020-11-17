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

#include <vector>
#include <string>
#include <utility>

namespace basecode {

    struct location_t {
        int32_t line = 0;
        int32_t column = 0;
    };

    struct source_location_t {
        location_t end{};
        location_t start{};
    };

    class result_message_t final {
    public:
        enum types {
            info,
            error,
            warning,
            data
        };

        result_message_t() = default;

        result_message_t(
                std::string_view code,
                std::string_view message,
                const source_location_t& loc = {},
                std::string_view details = "",
                types type = types::info) : _type(type),
                                            _code(code),
                                            _message(message),
                                            _details(details),
                                            _location(loc) {
        }

        [[nodiscard]] inline types type() const {
            return _type;
        }

        [[nodiscard]] inline bool is_error() const {
            return _type == types::error;
        }

        [[nodiscard]] inline std::string_view code() const {
            return _code;
        }

        [[nodiscard]] inline std::string_view details() const {
            return _details;
        }

        [[nodiscard]] inline std::string_view message() const {
            return _message;
        }

        [[nodiscard]] inline const source_location_t& location() const {
            return _location;
        }

    private:
        types _type{};
        std::string_view _code{};
        std::string_view _message{};
        std::string_view _details{};
        source_location_t _location{};
    };

}