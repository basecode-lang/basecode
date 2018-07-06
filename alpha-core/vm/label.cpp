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

#include "label.h"

namespace basecode::vm {

    label::label(const std::string& name) : _name(name) {
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

};