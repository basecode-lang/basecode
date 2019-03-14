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

#include <vm/terp.h>
#include <fmt/format.h>
#include <vm/assembler.h>
#include <common/defer.h>
#include <common/bytes.h>
#include <compiler/session.h>
#include <compiler/elements.h>

namespace basecode::compiler {

    program::program() : element(nullptr, nullptr, element_type_t::program) {
    }

    compiler::block* program::block() {
        return _block;
    }

    void program::block(compiler::block* value) {
        _block = value;
    }

}