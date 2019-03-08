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

#include <unordered_map>
#include <common/id_pool.h>
#include "basic_block.h"

namespace basecode::vm {

    class basic_block_map {
    public:
        basic_block_map() = default;

        ~basic_block_map();

        void reset();

        basic_block* make();

        bool remove(common::id_t id);

        basic_block* find(common::id_t id) const;

    private:
        std::unordered_map<common::id_t, basic_block*> _blocks {};
    };

};

