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

#include "numeric_type.h"

namespace basecode::compiler {

    numeric_type::numeric_type(
            element* parent,
            const std::string& name,
            int64_t min,
            uint64_t max) : type(parent, name),
                            _min(min),
                            _max(max) {
    }

};