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
#include "type.h"
#include "identifier.h"
#include "argument_list.h"
#include "assembly_label.h"
#include "symbol_element.h"
#include "integer_literal.h"
#include "identifier_reference.h"
#include "address_of_intrinsic.h"

namespace basecode::compiler {

    address_of_intrinsic::address_of_intrinsic(
            compiler::module* module,
            block* parent_scope,
            argument_list* args,
            const compiler::type_reference_list_t& type_params) : intrinsic(module, parent_scope, args, type_params) {
    }

    bool address_of_intrinsic::on_fold(
            compiler::session& session,
            fold_result_t& result) {
        auto args = arguments()->elements();
        if (args.empty() || args.size() > 1) {
            session.error(
                this,
                "P091",
                "address_of expects a single argument.",
                location());
            return false;
        }

        auto arg = args[0];
        if (arg == nullptr
        ||  arg->element_type() != element_type_t::identifier_reference) {
            session.error(
                this,
                "P091",
                "address_of expects an identifier reference parameter.",
                location());
            return false;
        }

        auto ref = dynamic_cast<compiler::identifier_reference*>(arg);
        result.element = session.builder().make_assembly_label(
            parent_scope(),
            ref->identifier()->symbol()->name(),
            module());
        result.element->location(location());
        return true;
    }

    bool address_of_intrinsic::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.inferred_type = session.scope_manager().find_type(qualified_symbol_t {
            .name = "u64"
        });
        return true;
    }

    bool address_of_intrinsic::can_fold() const {
        return true;
    }

    std::string address_of_intrinsic::name() const {
        return "address_of";
    }

};