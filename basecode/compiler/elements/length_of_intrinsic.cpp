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
#include "identifier.h"
#include "array_type.h"
#include "initializer.h"
#include "pointer_type.h"
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

        auto arg = args[0];
        switch (arg->element_type()) {
            case element_type_t::identifier: {
                auto identifier = dynamic_cast<compiler::identifier*>(arg);
                return length_of_identifier(session, identifier, result);
            }
            case element_type_t::string_literal: {
                auto string_literal = dynamic_cast<compiler::string_literal*>(arg);
                result.element = session.builder().make_integer(
                    parent_scope(),
                    string_literal->value().size());
                return true;
            }
            case element_type_t::identifier_reference: {
                auto ref = dynamic_cast<compiler::identifier_reference*>(arg);
                return length_of_identifier(session, ref->identifier(), result);
            }
            default: {
                infer_type_result_t type_result {};
                if (arg->infer_type(session, type_result)) {
                    switch (type_result.inferred_type->element_type()) {
                        case element_type_t::array_type: {
                            // XXX: revisit after fixing array_type
                            //auto array_type = dynamic_cast<compiler::array_type*>(type_result.inferred_type);
                            result.element = session.builder().make_integer(
                                parent_scope(),
                                0);
                            return true;
                        }
                        case element_type_t::pointer_type: {
                            auto pointer_type = dynamic_cast<compiler::pointer_type*>(type_result.inferred_type);
                            if (pointer_type->base_type_ref()->type()->element_type() == element_type_t::numeric_type) {
                                // XXX: this isn't going to work, is it?
                            }
                            break;
                        }
                        default: {
                            session.error(
                                module(),
                                "X000",
                                "length_of supports string literals and fixed arrays.",
                                location());
                            break;
                        }
                    }
                } else {
                    session.error(
                        module(),
                        "X000",
                        "length_of cannot infer argument type.",
                        location());
                }
                break;
            }
        }

        return false;
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
        return true;
    }

    std::string length_of_intrinsic::name() const {
        return "length_of";
    }

    bool length_of_intrinsic::length_of_identifier(
            compiler::session& session,
            compiler::identifier* identifier,
            fold_result_t& result) {
        if (identifier == nullptr)
            return false;

        auto init = identifier->initializer();
        if (init != nullptr) {
            auto expr = init->expression();
            auto string_literal = dynamic_cast<compiler::string_literal*>(expr);
            if (string_literal != nullptr) {
                result.element = session.builder().make_integer(
                    parent_scope(),
                    string_literal->value().size());
                return true;
            }
        }

        return false;
    }

    bool length_of_intrinsic::on_is_constant() const {
        return true;
    }

};