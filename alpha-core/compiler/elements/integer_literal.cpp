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

#include <common/bytes.h>
#include <compiler/session.h>
#include <vm/instruction_block.h>
#include "program.h"
#include "numeric_type.h"
#include "integer_literal.h"

namespace basecode::compiler {

    integer_literal::integer_literal(
            compiler::module* module,
            block* parent_scope,
            uint64_t value) : element(module, parent_scope, element_type_t::integer_literal),
                              _value(value) {
    }

    bool integer_literal::on_infer_type(
            const compiler::session& session,
            type_inference_result_t& result) {
        result.type = session.scope_manager().find_type({
            .name = numeric_type::narrow_to_value(_value)
        });
        return result.type != nullptr;
    }

    bool integer_literal::is_signed() const {
        return common::is_sign_bit_set(_value);
    }

    uint64_t integer_literal::value() const {
        return _value;
    }

    bool integer_literal::on_is_constant() const {
        return true;
    }

    bool integer_literal::on_emit(compiler::session& session) {
        auto instruction_block = session.assembler().current_block();
        auto target_reg = session.assembler().current_target_register();
        instruction_block->move_constant_to_reg(*target_reg, _value);
        return true;
    }

    bool integer_literal::on_as_integer(uint64_t& value) const {
        value = _value;
        return true;
    }

};