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
#include "program.h"
#include "integer_literal.h"

namespace basecode::compiler {

    integer_literal::integer_literal(
            element* parent,
            uint64_t value) : element(parent, element_type_t::integer_literal),
                              _value(value) {
    }

    uint64_t integer_literal::value() const {
        return _value;
    }

    bool integer_literal::on_as_integer(uint64_t& value) const {
        value = _value;
        return true;
    }

    compiler::type* integer_literal::on_infer_type(const compiler::program* program) {
        // XXX: i'm a bad person, i should do type narrowing here
        return program->find_type_up("u32");
    }

    bool integer_literal::on_emit(common::result& r, vm::assembler& assembler) {
        auto instruction_block = assembler.current_block();
        auto dest_reg = instruction_block->allocate_ireg();
        instruction_block->move_u32_to_ireg(dest_reg, static_cast<uint32_t>(_value));
        instruction_block->free_ireg(dest_reg);
        return true;
    }

};