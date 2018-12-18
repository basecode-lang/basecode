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

#include <vm/assembler.h>
#include <compiler/session.h>
#include <vm/instruction_block.h>
#include "assembly_label.h"
#include "identifier.h"
#include "symbol_element.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    assembly_label::assembly_label(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::identifier_reference* ref,
            const std::string& name) : element(module, parent_scope, element_type_t::assembly_label),
                                       _name(name),
                                       _ref(ref) {
    }

    bool assembly_label::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        auto& assembler = session.assembler();

        if (_ref != nullptr) {
            variable_handle_t temp_var {};
            if (!session.variable(_ref->identifier(), temp_var))
                return false;
            if (!temp_var->address_of())
                return false;
            result.operands.emplace_back(temp_var->emit_result().operands.front());
        } else {
            auto label_ref = assembler.make_label_ref(_name);
            result.operands.emplace_back(vm::instruction_operand_t(label_ref));
        }

        return true;
    }

    bool assembly_label::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.inferred_type = session
            .scope_manager()
            .find_type(qualified_symbol_t("u64"));
        return true;
    }

    std::string assembly_label::name() const {
        return _name;
    }

    bool assembly_label::on_is_constant() const {
        return true;
    }

    compiler::identifier_reference* assembly_label::reference() {
        return _ref;
    }

};