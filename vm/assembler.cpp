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

#include "assembler.h"

namespace basecode::vm {

    assembler::assembler() {
    }

    assembler::~assembler() {
    }

    bool assembler::assemble(
            common::result& r,
            vm::terp& terp,
            std::istream& source,
            uint64_t address) {
        return false;
    }

};