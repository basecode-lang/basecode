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

namespace basecode::compiler {

    label::label(
            element* parent,
            const std::string& name) : element(parent, element_type_t::label),
                                       _name(name) {
    }

    std::string label::name() const {
        return _name;
    }

};