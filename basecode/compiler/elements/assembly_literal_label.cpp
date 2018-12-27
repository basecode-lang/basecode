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
#include "assembly_literal_label.h"

namespace basecode::compiler {

    assembly_literal_label::assembly_literal_label(
        compiler::module* module,
        compiler::block* parent_scope,
        compiler::type* type,
        const std::string& name) : element(module, parent_scope, element_type_t::assembly_literal_label),
                                   _name(name),
                                   _type(type) {
    }

    bool assembly_literal_label::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        auto& assembler = session.assembler();

        auto label_ref = assembler.make_label_ref(_name);
        result.operands.emplace_back(vm::instruction_operand_t(label_ref));

        return true;
    }

    bool assembly_literal_label::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.inferred_type = _type;
        return true;
    }

    std::string assembly_literal_label::name() const {
        return _name;
    }

    bool assembly_literal_label::on_is_constant() const {
        return true;
    }

    compiler::type* assembly_literal_label::type() const {
        return _type;
    }

};