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
#include "run_directive.h"

namespace basecode::compiler {

    run_directive::run_directive(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* expression) : directive(module, parent_scope, "run"),
                                             _expression(expression) {
    }

//    bool run_directive::on_emit(
//            compiler::session& session,
//            compiler::emit_context_t& context,
//            compiler::emit_result_t& result) {
//        auto& assembler = session.assembler();
//        auto block = assembler.current_block();
//
//        block->comment(
//            "directive: run",
//            vm::comment_location_t::after_instruction);
//        block->meta_begin();
//        auto success = _expression->emit(session, context, result);
//        block->meta_end();
//
//        return success;
//    }

    bool run_directive::on_execute(compiler::session& session) {
        session.enable_run();
        return true;
    }

    void run_directive::on_owned_elements(element_list_t& list) {
        list.emplace_back(_expression);
    }

};