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
#include <compiler/scope_manager.h>
#include "type.h"
#include "identifier.h"
#include "pointer_type.h"
#include "argument_list.h"
#include "assembly_label.h"
#include "symbol_element.h"
#include "integer_literal.h"
#include "identifier_reference.h"
#include "address_of_intrinsic.h"

namespace basecode::compiler {

    address_of_intrinsic::address_of_intrinsic(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::argument_list* args,
            compiler::procedure_type* proc_type,
            const compiler::type_reference_list_t& type_params) : intrinsic(module,
                                                                            parent_scope,
                                                                            args,
                                                                            proc_type,
                                                                            type_params) {
    }

    bool address_of_intrinsic::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        auto args = arguments()->elements();

        variable_handle_t temp_var {};
        if (!session.variable(args[0], temp_var))
            return false;

        if (!temp_var->address_of())
            return false;

        result.operands.emplace_back(temp_var->emit_result().operands.front());
        return true;
    }

    bool address_of_intrinsic::on_fold(
            compiler::session& session,
            fold_result_t& result) {
        auto args = arguments()->elements();
        if (args.empty() || args.size() > 1) {
            session.error(
                module(),
                "P091",
                "address_of expects a single argument.",
                location());
            return false;
        }

        auto arg = args[0];
        if (arg == nullptr
        ||  arg->element_type() != element_type_t::identifier_reference) {
            session.error(
                module(),
                "P091",
                "address_of expects an identifier reference parameter.",
                location());
            return false;
        }

        auto ref = dynamic_cast<compiler::identifier_reference*>(arg);
        result.element = session.builder().make_assembly_label(
            parent_scope(),
            ref,
            nullptr,
            {},
            module());
        result.element->location(location());

        arguments()->clear();

        return true;
    }

    bool address_of_intrinsic::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        auto args = arguments()->elements();
        if (args.empty() || args.size() > 1)
            return false;

        infer_type_result_t base_result {};
        if (!args[0]->infer_type(session, base_result))
            return false;

        auto& builder = session.builder();
        auto& scope_manager = session.scope_manager();

        auto type = scope_manager.find_pointer_type(base_result.inferred_type);
        if (type == nullptr) {
            type = builder.make_pointer_type(
                parent_scope(),
                qualified_symbol_t(),
                base_result.inferred_type);
        }

        result.inferred_type = type;
        return true;
    }

    bool address_of_intrinsic::can_fold() const {
        return is_constant_parameter();
    }

    std::string address_of_intrinsic::name() const {
        return "address_of";
    }

    bool address_of_intrinsic::is_constant_parameter() const {
        auto args = arguments()->elements();
        if (args.empty() || args.size() > 1)
            return false;

        auto arg = args[0];
        return arg != nullptr
            && arg->element_type() == element_type_t::identifier_reference;
    }

};