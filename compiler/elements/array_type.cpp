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


#include "array_type.h"

namespace basecode::compiler {

    array_type::array_type(
            const std::string& name,
            compiler::type* element_type) : type(name),
                                            _element_type(element_type) {
    }

};