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

#include "return_element.h"

namespace basecode::compiler {

    return_element::return_element(element* parent) : element(parent, element_type_t::return_e) {
    }

    element_list_t& return_element::expressions() {
        return _expressions;
    }

};