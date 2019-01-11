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

#include <vm/instruction_block.h>
#include <common/string_support.h>
#include "session.h"
#include "string_intern_map.h"
#include "elements/string_literal.h"

namespace basecode::compiler {

    void string_intern_map::reset() {
        _interned_strings.clear();
        _element_to_intern_ids.clear();
    }

    bool string_intern_map::id(
            compiler::string_literal* literal,
            common::id_t& intern_id) {
        auto it = _element_to_intern_ids.find(literal->id());
        if (it == _element_to_intern_ids.end())
            return false;
        intern_id = it->second;
        return true;
    }

    intern_string_map_t::const_iterator string_intern_map::end() const {
        return _interned_strings.end();
    }

    intern_string_map_t::const_iterator string_intern_map::begin() const {
        return _interned_strings.begin();
    }

    std::string string_intern_map::base_label_for_id(common::id_t id) const {
        return fmt::format("_intern_str_lit_{}", id);
    }

    std::string string_intern_map::data_label_for_id(common::id_t id) const {
        return fmt::format("_intern_str_lit_{}_data", id);
    }

    common::id_t string_intern_map::intern(compiler::string_literal* literal) {
        auto it = _interned_strings.find(literal->value());
        if (it != _interned_strings.end()) {
            auto eit = _element_to_intern_ids.find(literal->id());
            if (eit == _element_to_intern_ids.end()) {
                _element_to_intern_ids.insert(std::make_pair(
                    literal->id(),
                    it->second));
            }
            return it->second;
        }

        auto id = common::id_pool::instance()->allocate();
        _interned_strings.insert(std::make_pair(literal->value(), id));
        _element_to_intern_ids.insert(std::make_pair(literal->id(), id));

        return id;
    }

    std::string string_intern_map::base_label(compiler::string_literal* literal) const {
        auto it = _element_to_intern_ids.find(literal->id());
        if (it == _element_to_intern_ids.end())
            return "";
        return base_label_for_id(it->second);
    }

    std::string string_intern_map::data_label(compiler::string_literal* literal) const {
        auto it = _element_to_intern_ids.find(literal->id());
        if (it == _element_to_intern_ids.end())
            return "";
        return data_label_for_id(it->second);
    }

};
