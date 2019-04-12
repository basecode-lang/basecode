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

#include <common/string_support.h>
#include "session.h"
#include "string_intern_map.h"
#include "elements/string_literal.h"

namespace basecode::compiler {

    bool string_intern_map::id(
            compiler::string_literal* literal,
            common::id_t& intern_id) {
        auto it = _element_to_intern_ids.find(literal->id());
        if (it == _element_to_intern_ids.end())
            return false;
        intern_id = it->second;
        return true;
    }

    bool string_intern_map::string(
            common::id_t id,
            std::string_view& data) const {
        auto it = _id_to_view.find(id);
        if (it == _id_to_view.end())
            return false;
        data = it->second;
        return true;
    }

    void string_intern_map::reset() {
        _strings.clear();
        _id_to_view.clear();
        _element_to_intern_ids.clear();
    }

    bool string_intern_map::element_id_to_intern_id(
            common::id_t element_id,
            common::id_t& intern_id) const {
        auto it = _element_to_intern_ids.find(element_id);
        if (it == _element_to_intern_ids.end())
            return false;
        intern_id = it->second;
        return true;
    }

    intern_string_map_t::const_iterator string_intern_map::end() const {
        return _strings.end();
    }

    intern_string_map_t::const_iterator string_intern_map::begin() const {
        return _strings.begin();
    }

    common::id_t string_intern_map::intern(compiler::string_literal* literal) {
        auto it = _strings.find(literal->value());
        if (it != _strings.end()) {
            auto eit = _element_to_intern_ids.find(literal->id());
            if (eit == _element_to_intern_ids.end()) {
                _element_to_intern_ids.insert(std::make_pair(
                    literal->id(),
                    it->second));
            }
            return it->second;
        }

        auto id = common::id_pool::instance()->allocate();
        auto new_entry = _strings.insert(std::make_pair(literal->value(), id));
        _id_to_view.insert(std::make_pair(id, new_entry.first->first));
        _element_to_intern_ids.insert(std::make_pair(literal->id(), id));

        return id;
    }

}
