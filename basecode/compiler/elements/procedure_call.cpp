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

#include <vm/ffi.h>
#include <common/defer.h>
#include <compiler/session.h>
#include <vm/instruction_block.h>
#include "type.h"
#include "program.h"
#include "identifier.h"
#include "initializer.h"
#include "unknown_type.h"
#include "argument_list.h"
#include "symbol_element.h"
#include "procedure_type.h"
#include "procedure_call.h"
#include "type_reference.h"
#include "composite_type.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    procedure_call::procedure_call(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::argument_list* args,
            const compiler::type_reference_list_t& type_params,
            const compiler::identifier_reference_list_t& references) : element(module, parent_scope, element_type_t::proc_call),
                                                                       _arguments(args),
                                                                       _type_parameters(type_params),
                                                                       _references(references) {
    }

    bool procedure_call::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        auto label = _resolved_identifier_ref->label_name();

        compiler::type* return_type = nullptr;
        auto return_type_field = _resolved_proc_type->return_type();
        if (return_type_field != nullptr)
            return_type = return_type_field->identifier()->type_ref()->type();

        size_t target_size = 8;
        type_number_class_t target_number_class;
        auto target_type = vm::register_type_t::integer;
        if (return_type != nullptr) {
            target_number_class = return_type->number_class();
            target_size = return_type->size_in_bytes();
            target_type = target_number_class == type_number_class_t::integer ?
                          vm::register_type_t::integer :
                          vm::register_type_t::floating_point;
        }

        if (_arguments != nullptr)
            _arguments->emit(session, context, result);

        if (_resolved_proc_type->is_foreign()) {
            auto& ffi = session.ffi();

            auto func = ffi.find_function(_resolved_proc_type->foreign_address());
            if (func == nullptr) {
                session.error(
                    module(),
                    "X000",
                    fmt::format(
                        "unable to find foreign function by address: {}",
                        _resolved_proc_type->foreign_address()),
                    location());
                return false;
            }

            block->comment(
                fmt::format("call: {}", label),
                vm::comment_location_t::after_instruction);

            vm::instruction_operand_t address_operand(_resolved_proc_type->foreign_address());

            if (func->is_variadic()) {
                vm::function_value_list_t args {};
                if (!_arguments->as_ffi_arguments(session, args))
                    return false;

                auto signature_id = common::id_pool::instance()->allocate();
                func->call_site_arguments.insert(std::make_pair(signature_id, args));

                block->call_foreign(
                    address_operand,
                    vm::instruction_operand_t(
                        static_cast<uint64_t>(signature_id),
                        vm::op_sizes::dword));
            } else {
                block->call_foreign(address_operand);
            }
        } else {
            if (return_type != nullptr) {
                block->comment(
                    "return slot",
                    vm::comment_location_t::after_instruction);
                block->sub(
                    vm::instruction_operand_t::sp(),
                    vm::instruction_operand_t::sp(),
                    vm::instruction_operand_t(static_cast<uint64_t>(8), vm::op_sizes::byte));
            }

            block->comment(
                fmt::format("call: {}", label),
                vm::comment_location_t::after_instruction);
            block->call(assembler.make_label_ref(label));
        }

        if (return_type_field != nullptr) {
            vm::instruction_operand_t result_operand;
            if (!vm::instruction_operand_t::allocate(
                    assembler,
                    result_operand,
                    vm::op_size_for_byte_size(target_size),
                    target_type)) {
                return false;
            }
            result.operands.emplace_back(result_operand);
            block->pop(result_operand);
        }

        if (_arguments->allocated_size() > 0) {
            block->comment(
                "free stack space",
                vm::comment_location_t::after_instruction);
            block->add(
                vm::instruction_operand_t::sp(),
                vm::instruction_operand_t::sp(),
                vm::instruction_operand_t(_arguments->allocated_size(), vm::op_sizes::word));
        }

        return true;
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

    compiler::argument_list* procedure_call::arguments() {
        return _arguments;
    }

    compiler::procedure_type* procedure_call::procedure_type() {
        return _resolved_proc_type;
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

        if (is_parent_element(element_type_t::binary_operator)) {
            infer_type_result_t type_result {};
            if (parent_element()->infer_type(session, type_result)) {
                if (!type_result.inferred_type->is_unknown_type())
                    return_type = type_result.inferred_type;
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
            for (const auto& prepare_result : results) {
                for (const auto& msg : prepare_result.messages.messages())
                    session.error(module(), msg.code(), msg.message(), msg.location());
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

};