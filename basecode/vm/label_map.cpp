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

#include "label_map.h"

namespace basecode::vm {

    vm::label* label_map::make(
            const std::string& name,
            basic_block* block) {
        auto it = _labels.insert(std::make_pair(
            name,
            vm::label(name, block)));
        return &it.first->second;
    }

    const vm::label* label_map::find(const std::string& name) const {
        const auto it = _labels.find(name);
        if (it == _labels.end())
            return nullptr;
        return &it->second;
    }

}