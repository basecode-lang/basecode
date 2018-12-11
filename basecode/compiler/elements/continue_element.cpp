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
#include "continue_element.h"

namespace basecode::compiler {

    continue_element::continue_element(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::label* label) : element(module, parent_scope, element_type_t::continue_e),
                                      _label(label) {
    }

    bool continue_element::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        vm::label_ref_t* label_ref = nullptr;

        std::string label_name;
        if (_label != nullptr) {
            label_name = _label->name();
            label_ref = assembler.make_label_ref(_label->name());
        } else {
            if (context.flow_control == nullptr
            ||  context.flow_control->continue_label == nullptr) {
                session.error(
                    this,
                    "P081",
                    "no valid continue label on stack.",
                    location());
                return false;
            }
            label_ref = context.flow_control->continue_label;
            label_name = label_ref->name;
        }

        block->comment(
            fmt::format("continue: {}", label_name),
            vm::comment_location_t::after_instruction);
        block->jump_direct(label_ref);

        return true;
    }

    compiler::label* continue_element::label() {
        return _label;
    }

    void continue_element::on_owned_elements(element_list_t& list) {
        if (_label != nullptr)
            list.emplace_back(_label);
    }

};