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

    class numeric_type : public type {
    public:
        explicit numeric_type(const std::string& name);

        inline uint64_t min() const {
            return _min;
        }

        inline uint64_t max() const {
            return _max;
        }

    private:
        uint64_t _min;
        uint64_t _max;
    };

};

