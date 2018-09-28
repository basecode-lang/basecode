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
#include "return_element.h"

namespace basecode::compiler {

    return_element::return_element(
        compiler::module* module,
        block* parent_scope) : element(module, parent_scope, element_type_t::return_e) {
    }

    element_list_t& return_element::expressions() {
        return _expressions;
    }

    bool return_element::on_emit(compiler::session& session) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();
        if (!_expressions.empty()) {
            vm::register_t target_reg;
            if (!assembler.allocate_reg(target_reg)) {
            }
            assembler.push_target_register(target_reg);
            // XXX: temporarily, only the first return value
            _expressions.front()->emit(session);
            block->store_from_reg(
                vm::register_t::fp(),
                target_reg,
                8);
            assembler.pop_target_register();
            assembler.free_reg(target_reg);
        }
        block->rts();
        return true;
    }

    void return_element::on_owned_elements(element_list_t& list) {
        for (auto element : _expressions)
            list.emplace_back(element);
    }

};