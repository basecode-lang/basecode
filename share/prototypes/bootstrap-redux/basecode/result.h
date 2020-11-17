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
#include <vector>
#include <utility>
#include <basecode/adt/array.h>
#include "result_message.h"

namespace basecode {

    class result_t final {
    public:
        explicit result_t(
                memory::allocator_t* allocator = context::current()->allocator) : _messages(allocator) {
            assert(allocator);
        }

        inline void fail() {
            _success = false;
        }

        inline void succeed() {
            _success = true;
        }

        inline void info(
                std::string_view code,
                std::string_view message,
                const source_location_t& loc = {},
                std::string_view details = {}) {
            _messages.add(result_message_t{
                code,
                message,
                loc,
                details,
                result_message_t::types::info});
        }

        inline void error(
                std::string_view code,
                std::string_view message,
                const source_location_t& loc = {},
                std::string_view details = {}) {
            _messages.add(result_message_t{
                code,
                message,
                loc,
                details,
                result_message_t::types::error});
            fail();
        }

        inline void warning(
                std::string_view code,
                std::string_view message,
                const source_location_t& loc = {},
                std::string_view details = {}) {
            _messages.add(result_message_t{
                code,
                message,
                loc,
                details,
                result_message_t::types::warning});
        }

        void remove_code(std::string_view code) {
            for (auto it = _messages.begin(); it != _messages.end(); ++it) {
                if ((*it).code() == code)
                    it = _messages.erase(it);
            }
        }

        [[nodiscard]] inline bool is_failed() const {
            return !_success;
        }

        [[nodiscard]] inline bool has_code(std::string_view code) const {
            for (const auto& msg : _messages)
                if (msg.code() == code)
                    return true;
            return false;
        }

        [[nodiscard]] inline const adt::array_t<result_message_t>& messages() const {
            return _messages;
        }

        [[nodiscard]] inline const result_message_t* find_code(std::string_view code) const {
            for (const auto& _message : _messages) {
                if (_message.code() == code)
                    return &_message;
            }
            return nullptr;
        }

    private:
        bool _success = true;
        adt::array_t<result_message_t> _messages;
    };

}
