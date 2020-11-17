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

#include <set>
#include <cstddef>
#include <boost/numeric/interval.hpp>

namespace basecode::id {

    using type_t = uint32_t;
    using set_t = std::set<type_t>;

    using optional_t = std::optional<type_t>;

    class interval_t final {
    public:
        interval_t(type_t ll, type_t uu) : _value(ll, uu) {}

        bool operator<(const interval_t& s) const {
            return
                (_value.lower() < s._value.lower()) &&
                (_value.upper() < s._value.lower());
        }

        [[nodiscard]] type_t left() const { return _value.lower(); }

        [[nodiscard]] type_t right() const { return _value.upper(); }

    private:
        boost::numeric::interval<type_t> _value;
    };

    using interval_set_t = std::set<interval_t>;

    class pool_t final {
    public:
        pool_t();

        pool_t(const pool_t&) = delete;

        type_t allocate();

        void release(type_t id);

        bool mark_used(type_t id);

        bool mark_range(type_t start_id, type_t end_id);

        type_t allocate_from_range(type_t start_id, type_t end_id);

    private:
        interval_set_t _set;
    };

}