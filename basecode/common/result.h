// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <string>
#include <vector>
#include <utility>
#include "result_message.h"

namespace basecode::common {

    class result {
    public:
        result() = default;

        inline void fail() {
            _success = false;
        }

        inline void succeed() {
            _success = true;
        }

        inline void info(
                const std::string& code,
                const std::string& message,
                const source_location& loc = {},
                const std::string& details = {}) {
            _messages.emplace_back(
                code,
                message,
                loc,
                details,
                result_message::types::info);
        }

        inline void error(
                const std::string& code,
                const std::string& message,
                const source_location& loc = {},
                const std::string& details = {}) {
            _messages.emplace_back(
                    code,
                    message,
                    loc,
                    details,
                    result_message::types::error);
            fail();
        }

        inline void warning(
                const std::string& code,
                const std::string& message,
                const source_location& loc = {},
                const std::string& details = {}) {
            _messages.emplace_back(
                code,
                message,
                loc,
                details,
                result_message::types::warning);
        }

        inline bool is_failed() const {
            return !_success;
        }

        void remove_code(const std::string& code) {
            for (auto it = _messages.begin(); it != _messages.end(); ++it) {
                if ((*it).code() == code)
                    it = _messages.erase(it);
            }
        }

        inline const result_message_list& messages() const {
            return _messages;
        }

        inline bool has_code(const std::string& code) const {
            for (const auto& msg : _messages)
                if (msg.code() == code)
                    return true;
            return false;
        }

        inline const result_message* find_code(const std::string& code) const {
            for (auto it = _messages.begin(); it != _messages.end(); ++it) {
                if ((*it).code() == code)
                    return &(*it);
            }
            return nullptr;
        }

    private:
        bool _success = true;
        result_message_list _messages {};
    };

}
