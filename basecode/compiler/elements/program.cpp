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

#include <compiler/session.h>
#include <compiler/element_builder.h>
#include "block.h"
#include "program.h"

namespace basecode::compiler {

    program::program(
        compiler::module* module,
        compiler::block* parent_scope) : element(module, parent_scope, element_type_t::program) {
    }

    compiler::block* program::block() {
        return _block;
    }

    compiler::element* program::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        auto copy = session.builder().make_program(new_scope->module(), new_scope);
        copy->_block = _block->clone<compiler::block>(session, new_scope);
        return copy;
    }

    void program::block(compiler::block* value) {
        _block = value;
    }

}