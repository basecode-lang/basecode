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
#include "array_type.h"
#include "initializer.h"
#include "pointer_type.h"
#include "type_literal.h"
#include "argument_list.h"
#include "type_reference.h"
#include "string_literal.h"
#include "integer_literal.h"
#include "length_of_intrinsic.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    length_of_intrinsic::length_of_intrinsic(
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

    bool length_of_intrinsic::on_fold(
            compiler::session& session,
            fold_result_t& result) {
        auto args = arguments()->elements();
        if (args.empty() || args.size() > 1) {
            session.error(
                module(),
                "P091",
                "length_of expects a single argument.",
                location());
            return false;
        }
        return length_for_element(session, args[0], result);
    }

    bool length_of_intrinsic::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.inferred_type = session
            .scope_manager()
            .find_type(qualified_symbol_t("u32"));
        return true;
    }

    bool length_of_intrinsic::can_fold() const {
        const auto& args = arguments()->elements();
        if (args.empty() || args.size() > 1)
            return false;

        const auto arg = args[0];
        if (arg->element_type() == element_type_t::identifier_reference) {
            auto ref = dynamic_cast<compiler::identifier_reference*>(arg);
            auto type = ref->identifier()->type_ref()->type();
            if (type->is_array_type()) {
                auto array_type = dynamic_cast<compiler::array_type*>(type);
                if (!array_type->are_subscripts_constant())
                    return false;
            }
        }

        return true;
    }

    std::string length_of_intrinsic::name() const {
        return "length_of";
    }

    bool length_of_intrinsic::length_for_element(
            compiler::session& session,
            compiler::element* e,
            fold_result_t& result) {
        switch (e->element_type()) {
            case element_type_t::identifier: {
                auto identifier = dynamic_cast<compiler::identifier*>(e);
                auto init = identifier->initializer();
                if (init != nullptr) {
                    auto expr = init->expression();
                    if (expr != nullptr)
                        return length_for_element(session, expr, result);
                }

                auto type = identifier->type_ref()->type();
                if (type->is_array_type()) {
                    auto array_type = dynamic_cast<compiler::array_type*>(type);
                    result.element = session.builder().make_integer(
                        parent_scope(),
                        array_type->number_of_elements());
                }

                break;
            }
            case element_type_t::type_literal: {
                auto literal = dynamic_cast<compiler::type_literal*>(e);
                result.element = session.builder().make_integer(
                    parent_scope(),
                    literal->args()->size());
                break;
            }
            case element_type_t::string_literal: {
                auto literal = dynamic_cast<compiler::string_literal*>(e);
                result.element = session.builder().make_integer(
                    parent_scope(),
                    literal->value().size());
                break;
            }
            case element_type_t::identifier_reference: {
                auto ref = dynamic_cast<compiler::identifier_reference*>(e);
                return length_for_element(session, ref->identifier(), result);
            }
            default: {
                break;
            }
        }
        return true;
    }

    bool length_of_intrinsic::on_is_constant() const {
        return true;
    }

}