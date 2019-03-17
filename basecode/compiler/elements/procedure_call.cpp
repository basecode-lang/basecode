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

#include <utility>
#include <vm/ffi.h>
#include <common/defer.h>
#include <compiler/session.h>
#include <compiler/element_builder.h>
#include "type.h"
#include "block.h"
#include "identifier.h"
#include "initializer.h"
#include "unknown_type.h"
#include "argument_list.h"
#include "symbol_element.h"
#include "procedure_type.h"
#include "procedure_call.h"
#include "type_reference.h"
#include "composite_type.h"
#include "binary_operator.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    procedure_call::procedure_call(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::argument_list* args,
            compiler::type_reference_list_t type_params,
            compiler::identifier_reference_list_t references) : element(module, parent_scope, element_type_t::proc_call),
                                                                _arguments(args),
                                                                _type_parameters(std::move(type_params)),
                                                                _references(std::move(references)) {
    }

    bool procedure_call::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        auto type = procedure_type();
        if (type != nullptr) {
            if (type->return_type() != nullptr) {
                auto return_identifier = type->return_type()->identifier();
                result.inferred_type = return_identifier->type_ref()->type();
                result.reference = return_identifier->type_ref();
                return true;
            }
        } else {
            result.inferred_type = session
                .builder()
                .make_unknown_type(
                    parent_scope(),
                    session.builder().make_symbol(parent_scope(), "---"),
                    this);
            return true;
        }

        return false;
    }

    bool procedure_call::is_foreign() const {
        if (_arguments == nullptr)
            return false;
        return _arguments->is_foreign_call();
    }

    bool procedure_call::uniform_function_call() const {
        return _uniform_function_call;
    }

    compiler::argument_list* procedure_call::arguments() {
        return _arguments;
    }

    void procedure_call::uniform_function_call(bool value) {
        _uniform_function_call = value;
    }

    compiler::procedure_type* procedure_call::procedure_type() {
        if (_resolved_proc_type != nullptr)
            return _resolved_proc_type;
        if (_references.size() == 1) {
            auto identifier = _references.front()->identifier();
            if (identifier != nullptr) {
                auto type = identifier->type_ref()->type();
                return dynamic_cast<compiler::procedure_type*>(type);
            }
        }
        return nullptr;
    }

    compiler::identifier_reference* procedure_call::identifier() {
        return _resolved_identifier_ref;
    }

    void procedure_call::on_owned_elements(element_list_t& list) {
        if (_arguments != nullptr)
            list.emplace_back(_arguments);

        for (auto ref : _references)
            list.emplace_back(ref);

        for (auto type_param : _type_parameters)
            list.emplace_back(type_param);
    }

    bool procedure_call::resolve_overloads(compiler::session& session) {
        compiler::type* return_type = nullptr;

        if (is_parent_type_one_of({element_type_t::binary_operator})) {
            auto bin_op = dynamic_cast<compiler::binary_operator*>(parent_element());
            if (bin_op->operator_type() == operator_type_t::assignment) {
                infer_type_result_t type_result{};
                if (parent_element()->infer_type(session, type_result)) {
                    if (!type_result.inferred_type->is_unknown_type())
                        return_type = type_result.inferred_type;
                }
            }
        }

        size_t success_count = 0;
        size_t success_index = 0;
        std::vector<prepare_call_site_result_t> results {};
        for (auto ref : _references) {
            prepare_call_site_result_t result {};
            result.ref = ref;

            auto type = result.ref->identifier()->type_ref()->type();
            result.proc_type = dynamic_cast<compiler::procedure_type*>(type);
            if (result.proc_type == nullptr) {
                session.error(
                    module(),
                    "X000",
                    "unknown procedure type.",
                    location());
                return false;
            }

            if (result.proc_type->prepare_call_site(session, _arguments, result)) {
                if (return_type != nullptr) {
                    auto return_fld = result.proc_type->return_type();
                    if (return_fld != nullptr) {
                        auto proc_return_type = return_fld->identifier()->type_ref()->type();
                        if (proc_return_type->id() == return_type->id()) {
                            ++success_count;
                            success_index = results.size();
                        }
                    }
                } else {
                    ++success_count;
                    success_index = results.size();
                }
            }

            results.emplace_back(result);
        }

        if (success_count == 0) {
            auto had_prepare_messages = false;
            for (const auto& prepare_result : results) {
                for (const auto& msg : prepare_result.messages.messages()) {
                    had_prepare_messages = true;
                    session.error(module(), msg.code(), msg.message(), msg.location());
                }
            }

            if (!had_prepare_messages) {
                session.error(
                    module(),
                    "X000",
                    "ambiguous call site.",
                    location());
            }

            return false;
        }

        if (success_count > 1) {
            session.error(
                module(),
                "X000",
                "ambiguous call site.",
                location());
            return false;
        }

        auto& matched_result = results[success_index];

        _arguments->elements(matched_result.arguments);
        _arguments->argument_index(matched_result.index);
        _arguments->is_foreign_call(matched_result.proc_type->is_foreign());

        _resolved_identifier_ref = matched_result.ref;
        _resolved_proc_type = matched_result.proc_type;

        return true;
    }

    const compiler::type_reference_list_t& procedure_call::type_parameters() const {
        return _type_parameters;
    }

    const compiler::identifier_reference_list_t& procedure_call::references() const {
        return _references;
    }

    void procedure_call::references(const compiler::identifier_reference_list_t& refs) {
        _references = refs;
    }

}