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

#include <common/bytes.h>
#include <compiler/session.h>
#include <vm/instruction_block.h>
#include "block.h"
#include "field.h"
#include "element.h"
#include "program.h"
#include "identifier.h"
#include "declaration.h"
#include "initializer.h"
#include "argument_pair.h"
#include "argument_list.h"
#include "procedure_call.h"
#include "procedure_type.h"
#include "symbol_element.h"
#include "type_reference.h"
#include "integer_literal.h"
#include "binary_operator.h"

namespace basecode::compiler {

    procedure_type::procedure_type(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::block* scope,
            compiler::symbol_element* symbol) : compiler::type(
                                                      module,
                                                      parent_scope,
                                                      element_type_t::proc_type,
                                                      symbol),
                                                _scope(scope) {
    }

    bool procedure_type::emit_epilogue(
            session& session,
            emit_context_t& context,
            emit_result_t& result) {
        if (is_foreign())
            return true;

        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        if (!_has_return) {
            block->rts();
        }

        return true;
    }

    bool procedure_type::emit_prologue(
            session& session,
            emit_context_t& context,
            emit_result_t& result) {
        if (is_foreign())
            return true;

        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        auto procedure_label = symbol()->name();
        auto parent_init = parent_element_as<compiler::initializer>();
        if (parent_init != nullptr) {
            auto parent_var = parent_init->parent_element_as<compiler::identifier>();
            if (parent_var != nullptr) {
                procedure_label = parent_var->label_name();
            }
        }

        block->blank_line();
        block->align(vm::instruction_t::alignment);
        block->label(assembler.make_label(procedure_label));

        auto frame = _scope->stack_frame();

        auto& stack_offsets = frame->offsets();
        stack_offsets.locals = 8;
        if (_return_type != nullptr) {
            auto entry = frame->add(
                stack_frame_entry_type_t::return_slot,
                _return_type->identifier()->symbol()->name(),
                8);
            _return_type->identifier()->stack_frame_entry(entry);

            stack_offsets.return_slot = 16;
            stack_offsets.parameters = 24;
        } else {
            stack_offsets.parameters = 16;
        }

        auto fields = parameters().as_list();
        for (auto fld : fields) {
            auto var = fld->identifier();
            auto type = var->type_ref()->type();
            // XXX: if we change procedure_call to
            //      sub.qw sp, sp, {size}
            //
            //      and then store.x sp, {value}, offset
            //      we can use truer sizes within
            //      the 8-byte aligned stack block.
            //
            auto entry = frame->add(
                stack_frame_entry_type_t::parameter,
                var->symbol()->name(),
                common::align(type->size_in_bytes(), 8));
            var->stack_frame_entry(entry);
        }

        return true;
    }

    bool procedure_type::prepare_call_site(
            compiler::session& session,
            compiler::argument_list* args,
            compiler::prepare_call_site_result_t& result) const {
        result.arguments = args->elements();

        auto field_list = _parameters.as_list();
        if (field_list.empty()) {
            if (!result.arguments.empty()) {
                result.messages.error(
                    "X000",
                    "procedure declares no parameters.",
                    args->parent_element()->location());
                return false;
            }
            return true;
        }

        auto& builder = session.builder();

        if (result.arguments.size() < field_list.size())
            result.arguments.resize(field_list.size());

        element_list_t temp {};
        temp.resize(field_list.size());
        compiler::argument_list* variadic_args = nullptr;

        size_t index = 0;
        for (index = 0; index < field_list.size(); index++) {
            auto fld = field_list[index];
            auto fld_name = fld->identifier()->symbol()->name();
            if (fld->is_variadic()) {
                if (index < field_list.size() - 1) {
                    result.messages.error(
                        "X000",
                        fmt::format(
                            "variadic parameter only valid in final position: {}",
                            fld_name),
                            args->parent_element()->location());
                    return false;
                } else {
                    variadic_args = builder.make_argument_list(args->parent_scope());
                    variadic_args->parent_element(args);
                    temp[index] = variadic_args;
                }
            }
            result.index.insert(std::make_pair(fld_name, index));
        }

        auto last_field = field_list.back();
        for (index = 0; index < result.arguments.size(); index++) {
        _retry:
            auto arg = result.arguments[index];
            if (arg == nullptr)
                continue;

            if (last_field->is_variadic()
            &&  index >= field_list.size() - 1) {
                if (variadic_args == nullptr) {
                    result.messages.error(
                        "X000",
                        "no variadic parameter defined.",
                        args->parent_element()->location());
                    return false;
                }

                variadic_args->add(arg);
                if (result.arguments.size() == field_list.size()) {
                    result.arguments[index] = nullptr;
                } else {
                    if (index < result.arguments.size()) {
                        result.arguments.erase(result.arguments.begin() + index);
                    } else {
                        continue;
                    }
                }
                goto _retry;
            } else {
                if (arg->element_type() != element_type_t::argument_pair)
                    continue;

                auto arg_pair = dynamic_cast<compiler::argument_pair*>(arg);
                std::string key;
                if (arg_pair->lhs()->as_string(key)) {
                    auto it = result.index.find(key);
                    if (it != result.index.end()) {
                        temp[it->second] = arg_pair->rhs();
                        result.arguments.erase(result.arguments.begin() + index);
                        result.arguments.emplace_back(nullptr);
                        goto _retry;
                    } else {
                        result.messages.error(
                            "X000",
                            fmt::format("invalid procedure parameter: {}", key),
                            arg->location());
                        return false;
                    }
                }
            }
        }

        index = 0;
        for (auto fld : field_list) {
            auto param = result.arguments[index];
            if (param != nullptr) {
                infer_type_result_t type_result{};
                if (!param->infer_type(session, type_result)) {
                    result.messages.error(
                        "X000",
                        fmt::format(
                            "unable to infer type for parameter: {}",
                            fld->identifier()->symbol()->name()),
                        param->location());
                    return false;
                }

                auto type_ref = fld->identifier()->type_ref();
                if (!type_ref->type()->type_check(type_result.inferred_type)) {
                    result.messages.error(
                        "X000",
                        fmt::format(
                            "type mismatch: cannot assign {} to parameter {}.",
                            type_result.type_name(),
                            fld->identifier()->symbol()->name()),
                        param->location());
                    return false;
                }
            } else {
                if (temp[index] != nullptr) {
                    result.arguments[index] = temp[index];
                    goto _next;
                }

                auto init = fld->identifier()->initializer();
                if (init == nullptr) {
                    auto decl = fld->declaration();
                    if (decl != nullptr) {
                        auto assignment = decl->assignment();
                        if (assignment != nullptr) {
                            result.arguments[index] = assignment->rhs();
                            goto _next;
                        }
                    }

                    result.messages.error(
                        "X000",
                        fmt::format(
                            "missing required parameter: {}",
                            fld->identifier()->symbol()->name()),
                        args->parent_element()->location());

                    return false;
                }

                result.arguments[index] = init->expression();
            }

        _next:
            index++;
        }

        return true;
    }

    bool procedure_type::has_return() const {
        return _has_return;
    }

    bool procedure_type::is_foreign() const {
        return _is_foreign;
    }

    compiler::block* procedure_type::scope() {
        return _scope;
    }

    field_map_t& procedure_type::parameters() {
        return _parameters;
    }

    bool procedure_type::is_proc_type() const {
        return true;
    }

    void procedure_type::is_foreign(bool value) {
        _is_foreign = value;
    }

    void procedure_type::has_return(bool value) {
        _has_return = value;
    }

    bool procedure_type::on_is_constant() const {
        return true;
    }

    type_map_t& procedure_type::type_parameters() {
        return _type_parameters;
    }

    compiler::field* procedure_type::return_type() {
        return _return_type;
    }

    uint64_t procedure_type::foreign_address() const {
        return _foreign_address;
    }

    void procedure_type::foreign_address(uint64_t value) {
        _foreign_address = value;
    }

    procedure_instance_list_t& procedure_type::instances() {
        return _instances;
    }

    void procedure_type::return_type(compiler::field* value) {
        _return_type = value;
    }

    compiler::procedure_instance* procedure_type::instance_for(
            compiler::session& session,
            compiler::procedure_call* call) {
        // XXX: this is not complete.  for testing only
        if (_instances.empty())
            return nullptr;
        return _instances.back();
    }

    bool procedure_type::on_type_check(compiler::type* other) {
        if (other == nullptr)
            return false;

        // XXX: very temporary hack...
        return other->element_type() == element_type_t::proc_type;
    }

    type_access_model_t procedure_type::on_access_model() const {
        return type_access_model_t::pointer;
    }

    void procedure_type::on_owned_elements(element_list_t& list) {
        if (_scope != nullptr)
            list.emplace_back(_scope);

        if (_return_type != nullptr)
            list.emplace_back(_return_type);

        for (auto element : _parameters.as_list())
            list.emplace_back(element);
    }

    bool procedure_type::on_initialize(compiler::session& session) {
        return true;
    }

};