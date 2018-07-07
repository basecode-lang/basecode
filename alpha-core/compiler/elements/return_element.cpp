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

#include <vm/instruction_block.h>
#include "return_element.h"

namespace basecode::compiler {

    return_element::return_element(
        element* parent) : element(parent, element_type_t::return_e) {
    }

    element_list_t& return_element::expressions() {
        return _expressions;
    }

    bool return_element::on_emit(common::result& r, vm::assembler& assembler) {
        auto instruction_block = assembler.current_block();

        for (auto expr : _expressions) {
            // XXX: why is expr null?
            if (expr != nullptr)
                expr->emit(r, assembler);
        }

        instruction_block->rts();

        return true;
    }

};