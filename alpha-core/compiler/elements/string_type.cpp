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

#include "string_type.h"

namespace basecode::compiler {

    string_type::string_type(
        element* parent) : compiler::type(
                                parent,
                                element_type_t::string_type,
                                "string") {
    }

    bool string_type::on_initialize(common::result& r) {
        // string_type := struct {
        //      length:u32;
        //      capacity:u32;
        //      data:address;
        // }; 16 bytes
        
        size_in_bytes(16);
        return true;
    }

};