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
#include "string_literal.h"

namespace basecode::compiler {

    string_literal::string_literal(
            element* parent,
            const std::string& value) : element(parent, element_type_t::string_literal),
                                        _value(value) {
    }

    bool string_literal::on_emit(
            common::result& r,
            vm::assembler& assembler,
            emit_context_t& context) {
        auto instruction_block = assembler.current_block();
        auto target_reg = instruction_block->current_target_register();
        instruction_block->move_label_to_ireg(target_reg->reg.i, label_name());
        return true;
    }

    bool string_literal::on_is_constant() const {
        return true;
    }

    bool string_literal::on_as_string(std::string& value) const {
        value = _value;
        return true;
    }

    compiler::type* string_literal::on_infer_type(const compiler::program* program) {
        return program->find_type_up("string");
    }

}
