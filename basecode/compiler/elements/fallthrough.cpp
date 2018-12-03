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
#include <vm/instruction_block.h>
#include "label.h"
#include "fallthrough.h"

namespace basecode::compiler {

    fallthrough::fallthrough(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::label* label) : element(module, parent_scope, element_type_t::fallthrough),
                                      _label(label) {
    }

    compiler::label* fallthrough::label() {
        return _label;
    }

    bool fallthrough::on_emit(compiler::session& session) {
        auto& assembler = session.assembler();
        auto control_flow = assembler.current_control_flow();
        if (control_flow == nullptr) {
            // XXX: error
            return false;
        }
        control_flow->fallthrough = true;
        return true;
    }

    void fallthrough::on_owned_elements(element_list_t& list) {
        if (_label != nullptr)
            list.emplace_back(_label);
    }

};