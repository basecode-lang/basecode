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
#include "type.h"
#include "program.h"
#include "numeric_type.h"
#include "float_literal.h"

namespace basecode::compiler {

    float_literal::float_literal(
            compiler::module* module,
            block* parent_scope,
            double value) : element(module, parent_scope, element_type_t::float_literal),
                            _value(value) {
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

    bool float_literal::on_emit(compiler::session& session) {
        auto instruction_block = session.assembler().current_block();
        auto target_reg = session.assembler().current_target_register();
        instruction_block->move_constant_to_reg(*target_reg, _value);
        return true;
    }

    compiler::type* float_literal::on_infer_type(const compiler::session& session) {
        return session.scope_manager().find_type({
            .name = numeric_type::narrow_to_value(_value)
        });
    }

};