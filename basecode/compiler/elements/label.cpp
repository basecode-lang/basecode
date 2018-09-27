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

#include <vm/assembler.h>
#include <compiler/session.h>
#include "label.h"

namespace basecode::compiler {

    label::label(
            compiler::module* module,
            block* parent_scope,
            const std::string& name) : element(module, parent_scope, element_type_t::label),
                                       _name(name) {
    }

    std::string label::name() const {
        return _name;
    }

    bool label::on_is_constant() const {
        return true;
    }

    bool label::on_emit(compiler::session& session) {
        auto& assembler = session.assembler();
        auto instruction_block = assembler.current_block();
        instruction_block->blank_line();
        instruction_block->label(assembler.make_label(_name));
        return true;
    }

};