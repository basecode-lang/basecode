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

#include "label.h"
#include <utility>

namespace basecode::vm {

    label::label(
            std::string name,
            basic_block* block) : _name(std::move(name)),
                                  _block(block) {
    }

    basic_block* label::block() {
        return _block;
    }

    std::string label::name() const {
        return _name;
    }

    uint64_t label::address() const {
        return _address;
    }

    void label::address(uint64_t value) {
        _address = value;
    }

}