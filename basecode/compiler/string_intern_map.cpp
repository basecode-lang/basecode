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

    bool string_intern_map::emit(compiler::session& session) {
        auto& assembler = session.assembler();

        auto block = assembler.make_basic_block();
        block->blank_line();
        block->comment("interned string literals", 0);
        block->section(vm::section_t::ro_data);

        for (const auto& kvp : _interned_strings) {
            block->blank_line();
            block->align(4);
            block->comment(
                fmt::format("\"{}\"", kvp.first),
                0);
            block->string(
                assembler.make_label(base_label_for_id(kvp.second)),
                assembler.make_label(data_label_for_id(kvp.second)),
                common::escaped_string(kvp.first));
        }

        return true;
    }

    std::string string_intern_map::base_label_for_id(common::id_t id) const {
        return fmt::format("_intern_str_lit_{}", id);
    }

    std::string string_intern_map::data_label_for_id(common::id_t id) const {
        return fmt::format("_intern_str_lit_{}_data", id);
    }

    common::id_t string_intern_map::intern(compiler::string_literal* literal) {
        auto it = _interned_strings.find(literal->value());
        if (it != _interned_strings.end())
            return it->second;

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
