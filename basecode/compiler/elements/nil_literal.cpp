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
#include "nil_literal.h"

namespace basecode::compiler {

    nil_literal::nil_literal(
            compiler::module* module,
            block* parent_scope) : element(module, parent_scope, element_type_t::nil_literal) {
    }

    bool nil_literal::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        result.operands.emplace_back(vm::instruction_operand_t(static_cast<uint64_t>(0)));
        return true;
    }

    bool nil_literal::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.inferred_type = session.scope_manager().find_type({.name = "u64"});
        return true;
    }

    bool nil_literal::on_is_constant() const {
        return true;
    }

};