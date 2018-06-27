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

    block::block(
        block* parent,
        element_type_t type) : element(parent, type),
                               _emitter(0) {
    }

    type_map_t& block::types() {
        return _types;
    }

    block_list_t& block::blocks() {
        return _blocks;
    }

    comment_list_t& block::comments() {
        return _comments;
    }

    bool block::emit(common::result& r) {
        _emitter.clear();
        return false;
    }

    statement_list_t& block::statements() {
        return _statements;
    }

    identifier_map_t& block::identifiers() {
        return _identifiers;
    }

    vm::instruction_emitter* block::emitter() {
        return &_emitter;
    }

};