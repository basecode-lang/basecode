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

#include <basecode/format/format.h>
#include "fake_logger.h"

namespace basecode::logging {

    fake_logger_t::fake_logger_t() : _entries(context::current()->allocator) {
    }

    fake_logger_t::fake_logger_t(memory::allocator_t* allocator) : _entries(allocator) {
    }

    void fake_logger_t::clear() {
        _entries.clear();
    }

    bool fake_logger_t::empty() const {
        return _entries.empty();
    }

    size_t fake_logger_t::size() const {
        return _entries.size();
    }

    fake_log_entry_t& fake_logger_t::operator[] (size_t index) {
        return _entries[index];
    }

    void fake_logger_t::on_log(log_level_t level, std::string_view message) {
        format::print("[{}] {}\n", log_level_name(level), message);
        _entries.emplace(level, message);
    }

}
