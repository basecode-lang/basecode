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

        inline void add_message(
                const std::string& code,
                const std::string& message) {
            _messages.emplace_back(code, message, std::string(), result_message::types::info);
        }

        inline void add_message(
                const std::string& code,
                const std::string& message,
                bool error) {
            _messages.emplace_back(
                    code,
                    message,
                    std::string(),
                    error ? result_message::types::error : result_message::types::info);
            if (error)
                fail();
        }

        inline void add_message(
                const std::string& code,
                const std::string& message,
                const std::string& details,
                bool error) {
            _messages.emplace_back(
                    code,
                    message,
                    details,
                    error ? result_message::types::error : result_message::types::info);
            if (error)
                fail();
        }

        inline bool is_failed() const {
            return !_success;
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

};
