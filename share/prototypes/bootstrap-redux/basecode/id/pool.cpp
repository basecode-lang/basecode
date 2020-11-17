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

#include "pool.h"

namespace basecode::id {

    pool_t::pool_t() {
        _set.insert(interval_t(1, std::numeric_limits<type_t>::max()));
    }

    type_t pool_t::allocate() {
        interval_t first = *(_set.begin());
        auto id = first.left();
        _set.erase(_set.begin());
        if (first.left() + 1 <= first.right()) {
            _set.insert(interval_t(first.left() + 1 , first.right()));
        }
        return id;
    }

    void pool_t::release(type_t id) {
        auto it = _set.find(interval_t(id, id));
        if (it != _set.end() && it->left() <= id && it->right() > id) {
            return;
        }
        it = _set.upper_bound(interval_t(id, id));
        if (it == _set.end()) {
            return;
        } else {
            interval_t _interval = *(it);
            if (id + 1 != _interval.left()) {
                _set.insert(interval_t(id, id));
            } else {
                if (it != _set.begin()) {
                    auto it_2 = it;
                    --it_2;
                    if (it_2->right() + 1 == id) {
                        interval_t _interval2 = *(it_2);
                        _set.erase(it);
                        _set.erase(it_2);
                        _set.insert(
                            interval_t(_interval2.left(),
                                       _interval.right()));
                    } else {
                        _set.erase(it);
                        _set.insert(interval_t(id, _interval.right()));
                    }
                } else {
                    _set.erase(it);
                    _set.insert(interval_t(id, _interval.right()));
                }
            }
        }
    }

    bool pool_t::mark_used(type_t id) {
        auto it = _set.find(interval_t(id, id));
        if (it == _set.end()) {
            return false;
        } else {
            interval_t free_interval = *(it);
            _set.erase(it);
            if (free_interval.left() < id) {
                _set.insert(interval_t(free_interval.left(), id - 1));
            }
            if (id + 1 <= free_interval.right()) {
                _set.insert(interval_t(id + 1, free_interval.right()));
            }
            return true;
        }
    }

    bool pool_t::mark_range(type_t start_id, type_t end_id) {
        for (auto id = static_cast<size_t>(start_id);
             id < static_cast<size_t>(end_id);
             ++id) {
            auto success = mark_used(static_cast<type_t>(id));
            if (!success)
                return false;
        }
        return true;
    }

    type_t pool_t::allocate_from_range(type_t start_id, type_t end_id) {
        for (auto id = static_cast<size_t>(start_id);
             id < static_cast<size_t>(end_id);
             ++id) {
            auto success = mark_used(static_cast<type_t>(id));
            if (success)
                return static_cast<type_t>(id);
        }
        return 0;
    }

}