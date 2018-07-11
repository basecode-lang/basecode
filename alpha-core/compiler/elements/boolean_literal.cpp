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
#include "boolean_literal.h"

namespace basecode::compiler {

    boolean_literal::boolean_literal(
            element* parent,
            bool value) : element(parent, element_type_t::boolean_literal),
                          _value(value) {
    }

    bool boolean_literal::on_emit(
            common::result& r,
            emit_context_t& context) {
        auto instruction_block = context.assembler->current_block();
        auto target_reg = instruction_block->current_target_register();
        instruction_block->move_u8_to_ireg(
            target_reg->reg.i,
            static_cast<uint8_t>(_value ? 1 : 0));
        return true;
    }

    bool boolean_literal::value() const {
        return _value;
    }

    bool boolean_literal::on_is_constant() const {
        return true;
    }

    bool boolean_literal::on_as_bool(bool& value) const {
        value = _value;
        return true;
    }

    compiler::type* boolean_literal::on_infer_type(const compiler::program* program) {
        return program->find_type_up("bool");
    }

};