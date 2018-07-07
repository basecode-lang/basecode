// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#include <vm/instruction_block.h>
#include "argument_list.h"

namespace basecode::compiler {

    argument_list::argument_list(
        element* parent) : element(parent, element_type_t::argument_list) {
    }

    bool argument_list::on_emit(
            common::result& r,
            vm::assembler& assembler) {
        auto instruction_block = assembler.current_block();
        for (auto arg : _elements) {
            uint64_t value = 0;
            instruction_block->push_u8(value);
        }
        return true;
    }

    void argument_list::add(element* item) {
        _elements.emplace_back(item);
    }

    void argument_list::remove(common::id_t id) {
        auto item = find(id);
        if (item == nullptr)
            return;
        std::remove(
            _elements.begin(),
            _elements.end(),
            item);
    }

    element* argument_list::find(common::id_t id) {
        auto it = std::find_if(
            _elements.begin(),
            _elements.end(),
            [&id](auto item) { return item->id() == id; });
        if (it == _elements.end())
            return nullptr;
        return *it;
    }

    const element_list_t& argument_list::elements() const {
        return _elements;
    }

};