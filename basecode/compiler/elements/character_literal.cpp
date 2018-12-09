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
#include "program.h"
#include "character_literal.h"

namespace basecode::compiler {
    
    character_literal::character_literal(
        compiler::module* module,
        block* parent_scope,
        common::rune_t rune) : element(module, parent_scope, element_type_t::character_literal),
                               _rune(rune) {
    }

    bool character_literal::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        result.operands.emplace_back(vm::instruction_operand_t(
            static_cast<int64_t>(_rune),
            vm::op_sizes::dword));
        return true;
    }

    bool character_literal::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.inferred_type = session
            .scope_manager()
            .find_type(qualified_symbol_t("rune"));
        return true;
    }

    bool character_literal::on_is_constant() const {
        return true;
    }

    common::rune_t character_literal::rune() const {
        return _rune;
    }

    bool character_literal::on_as_rune(common::rune_t& value) const {
        value = _rune;
        return true;
    }

};