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

#pragma once

#include <string>
#include <unordered_map>
#include "label.h"

namespace basecode::vm {

    enum class deferred_edge_type_t {
        predecessor,
        successor
    };

    struct deferred_edge_t {
        deferred_edge_type_t type {};
        basic_block* block = nullptr;
    };

    class label_map {
    public:
        label_map() = default;

        vm::label* make(
            const std::string& name,
            basic_block* block);

        const vm::label* find(const std::string& name) const;

    private:
        std::unordered_map<std::string, vm::label> _labels {};
    };

}

