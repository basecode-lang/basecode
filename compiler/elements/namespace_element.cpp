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

#include "namespace_element.h"

namespace basecode::compiler {

    namespace_element::namespace_element(
        block* parent,
        const std::string& name) : block(parent, element_type_t::namespace_e),
                                   _name(name) {
    }

    std::string namespace_element::name() const {
        return _name;
    }

};