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
#include "float_literal.h"

namespace basecode::compiler {

    float_literal::float_literal(
            element* parent,
            double value) : element(parent, element_type_t::float_literal),
                            _value(value) {
    }

    bool float_literal::on_emit(
            common::result& r,
            emit_context_t& context) {
        auto instruction_block = context.assembler->current_block();
        auto target_reg = instruction_block->current_target_register();
        instruction_block->move_u64_to_ireg(
            target_reg->reg.i,
            static_cast<uint64_t>(_value));
        return true;
    }

    double float_literal::value() const {
        return _value;
    }

    bool float_literal::on_is_constant() const {
        return true;
    }

    bool float_literal::on_as_float(double& value) const {
        value = _value;
        return true;
    }

    compiler::type* float_literal::on_infer_type(const compiler::program* program) {
        return program->find_type_up("f64");
    }

};