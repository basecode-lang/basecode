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
#include "basic_block_map.h"

namespace basecode::vm {

    basic_block_map::~basic_block_map() {
        reset();
    }

    void basic_block_map::reset() {
        for (const auto& kvp : _blocks)
            delete kvp.second;
        _blocks.clear();
    }

    basic_block* basic_block_map::make() {
        auto block = new basic_block(basic_block_type_t::none);
        _blocks.insert(std::make_pair(block->id(), block));
        return block;
    }

    bool basic_block_map::remove(common::id_t id) {
        auto block = find(id);
        if (block == nullptr)
            return false;
        delete block;
        return _blocks.erase(id) > 0;
    }

    basic_block_list_t basic_block_map::as_list() const {
        basic_block_list_t list{};
        for (const auto& kvp : _blocks)
            list.push_back(kvp.second);
        return list;
    }

    basic_block* basic_block_map::find(common::id_t id) const {
        auto it = _blocks.find(id);
        if (it == std::end(_blocks))
            return nullptr;
        return it->second;
    }

}