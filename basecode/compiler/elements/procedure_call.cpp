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

        auto label = _active_identifier->label_name();

        compiler::type* return_type = nullptr;
        auto return_type_field = _active_procedure_type->return_type();
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

        if (_active_procedure_type->is_foreign()) {
            auto& ffi = session.ffi();

            auto func = ffi.find_function(_active_procedure_type->foreign_address());
            if (func == nullptr) {
                session.error(
                    module(),
                    "X000",
                    fmt::format(
                        "unable to find foreign function by address: {}",
                        _active_procedure_type->foreign_address()),
                    location());
                return false;
            }

            block->comment(
                fmt::format("call: {}", label),
                vm::comment_location_t::after_instruction);

            vm::instruction_operand_t address_operand(_active_procedure_type->foreign_address());

            if (func->is_variadic()) {
                auto signature_id = common::id_pool::instance()->allocate();

                vm::function_value_list_t args {};
                std::function<bool (const element_list_t&)> recurse_arguments =
                    [&](const element_list_t& elements) -> bool {
                        for (auto arg : elements) {
                            if (arg->element_type() == element_type_t::argument_list) {
                                auto arg_list = dynamic_cast<compiler::argument_list*>(arg);
                                auto success = recurse_arguments(arg_list->elements());
                                if (!success)
                                    return false;
                                continue;
                            }

                            infer_type_result_t type_result {};
                            if (!arg->infer_type(session, type_result))
                                return false;

                            vm::function_value_t value {};
                            value.type = type_result.inferred_type->to_ffi_type();
                            if (value.type == vm::ffi_types_t::struct_type) {
                                // XXX: temporary hack to keep string literals working
                                if (type_result.inferred_type->element_type() == element_type_t::string_type) {
                                    value.type = vm::ffi_types_t::pointer_type;
                                } else {
                                    auto composite_type = dynamic_cast<compiler::composite_type*>(type_result.inferred_type);
                                    if (composite_type == nullptr)
                                        return false;
                                    for (auto fld : composite_type->fields().as_list()) {
                                        vm::function_value_t fld_value;
                                        fld_value.name = fld->identifier()->symbol()->name();
                                        fld_value.type = fld->identifier()->type_ref()->type()->to_ffi_type();
                                        value.fields.push_back(fld_value);
                                    }
                                }
                            }
                            args.push_back(value);
                        }
                        return true;
                    };

                if (!recurse_arguments(_arguments->elements()))
                    return false;

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
        // XXX: temporary work around
        if (_references.size() == 1) {
            _active_identifier = _references.front();

            auto type = _active_identifier->identifier()->type_ref()->type();
            _active_procedure_type = dynamic_cast<compiler::procedure_type*>(type);
        }
        return _active_procedure_type;
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
        auto type = procedure_type();

        if (type != nullptr
        && !_arguments->index_to_procedure_type(session, type)) {
            return false;
        }

        _active_identifier = _references.front();
        auto proc_type = _active_identifier->identifier()->type_ref()->type();
        _active_procedure_type = dynamic_cast<compiler::procedure_type*>(proc_type);

        return true;
    }

    const compiler::type_reference_list_t& procedure_call::type_parameters() const {
        return _type_parameters;
    }

    const compiler::identifier_reference_list_t& procedure_call::references() const {
        return _references;
    }

};