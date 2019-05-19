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
#include <compiler/scope_manager.h>
#include <compiler/element_builder.h>
#include "type.h"
#include "block.h"
#include "identifier.h"
#include "initializer.h"
#include "pointer_type.h"
#include "argument_list.h"
#include "assembly_label.h"
#include "symbol_element.h"
#include "type_reference.h"
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
        auto init = ref->identifier()->initializer();
        if (init != nullptr) {
            auto init_expr = init->expression();
            if (init_expr != nullptr && init_expr->is_type()) {
                session.error(
                    module(),
                    "X000",
                    "address_of does not support type identifiers; use type_of instead.",
                    location());
                return false;
            }
        }

        result.element = session.builder().make_assembly_label(parent_scope(), ref);
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

        const auto& inferred = base_result.types.back();
        auto& builder = session.builder();
        auto& scope_manager = session.scope_manager();

        auto type = scope_manager.find_pointer_type(inferred.type);
        if (type == nullptr) {
            auto it = session.strings().insert(compiler::pointer_type::name_for_pointer(inferred.type));
            type = builder.make_pointer_type(
                parent_scope(),
                qualified_symbol_t(*it.first),
                inferred.type);
        }

        result.types.emplace_back(type);
        return true;
    }

    bool address_of_intrinsic::can_fold() const {
        return is_constant_parameter();
    }

    bool address_of_intrinsic::on_is_constant() const {
        return is_constant_parameter();
    }

    intrinsic_type_t address_of_intrinsic::type() const {
        return intrinsic_type_t::address_of;
    }

    bool address_of_intrinsic::is_constant_parameter() const {
        auto args = arguments()->elements();
        if (args.empty() || args.size() > 1)
            return false;

        // XXX: STACK FRAME check was here, do we still need that?
        auto arg = args[0];
        return arg != nullptr
            && arg->element_type() == element_type_t::identifier_reference;
    }

}