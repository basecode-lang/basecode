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

#pragma once

#include "type.h"

namespace basecode::compiler {

    class array_type : public type {
    public:
        array_type(
            element* parent,
            const std::string& name,
            compiler::type* element_type);

        uint64_t size() const {
            return _size;
        }

        void size(uint64_t value) {
            _size = value;
        }

        compiler::type* element_type() {
            return _element_type;
        }

    private:
        uint64_t _size = 0;
        compiler::type* _element_type = nullptr;
    };

};

