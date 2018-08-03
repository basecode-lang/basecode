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

#include <vm/instruction_block.h>
#include "type.h"
#include "program.h"
#include "numeric_type.h"
#include "float_literal.h"

namespace basecode::compiler {

    float_literal::float_literal(
            block* parent_scope,
            double value) : element(parent_scope, element_type_t::float_literal),
                            _value(value) {
    }

    bool float_literal::on_emit(
            common::result& r,
            emit_context_t& context) {
        auto instruction_block = context.assembler->current_block();
        auto target_reg = context.assembler->current_target_register();
        auto inferred_type = infer_type(context.program);
        instruction_block->move_constant_to_freg(
            vm::op_size_for_byte_size(inferred_type->size_in_bytes()),
            target_reg->reg.f,
            _value);
        return true;
    }

    double float_literal::value() const {
        return _value;
    }

    bool float_literal::is_signed() const {
        return _value < 0;
    }

    bool float_literal::on_is_constant() const {
        return true;
    }

    bool float_literal::on_as_float(double& value) const {
        value = _value;
        return true;
    }

    compiler::type* float_literal::on_infer_type(const compiler::program* program) {
        return program->find_type({
            .name = numeric_type::narrow_to_value(_value)
        });
    }

};