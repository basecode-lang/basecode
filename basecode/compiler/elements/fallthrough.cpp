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

//    bool fallthrough::on_emit(
//            compiler::session& session,
//            compiler::emit_context_t& context,
//            compiler::emit_result_t& result) {
//        if (context.flow_control == nullptr) {
//            // XXX: error
//            return false;
//        }
//        context.flow_control->fallthrough = true;
//        return true;
//    }

    compiler::label* fallthrough::label() {
        return _label;
    }

    void fallthrough::on_owned_elements(element_list_t& list) {
        if (_label != nullptr)
            list.emplace_back(_label);
    }

};