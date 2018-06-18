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

#include "block.h"

namespace basecode::compiler {

    block::block(block* parent) : element(parent) {
    }

    block::~block() {
    }

    element_list_t& block::children() {
        return _children;
    }

};