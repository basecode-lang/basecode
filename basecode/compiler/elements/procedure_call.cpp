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

#include <common/defer.h>
#include <compiler/session.h>
#include <vm/instruction_block.h>
#include "type.h"
#include "program.h"
#include "identifier.h"
#include "initializer.h"
#include "argument_list.h"
#include "symbol_element.h"
#include "procedure_type.h"
#include "procedure_call.h"
#include "type_reference.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    procedure_call::procedure_call(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::identifier_reference* reference,
            compiler::argument_list* args,
            const compiler::type_reference_list_t& type_params) : element(module, parent_scope, element_type_t::proc_call),
                                                                  _arguments(args),
                                                                  _type_parameters(type_params),
                                                                  _reference(reference) {
    }

    bool procedure_call::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        auto& assembler = session.assembler();

        auto block = assembler.current_block();
        auto identifier = _reference->identifier();
        auto init = identifier->initializer();
        if (init == nullptr)
            return false;

        auto procedure_type = init->procedure_type();

        if (_arguments != nullptr)
            _arguments->emit(session, context, result);

        if (procedure_type->is_foreign()) {
            block->comment(
                fmt::format("foreign call: {}", identifier->symbol()->name()),
                4);
            vm::instruction_operand_t arg_count(
                static_cast<uint64_t>(_arguments->size()),
                vm::op_sizes::word);
            block->push(arg_count);
            block->call_foreign(procedure_type->foreign_address());
        } else {
            block->call(assembler.make_label_ref(identifier->symbol()->name()));
        }

        auto return_type_field = procedure_type->return_type();
        if (return_type_field != nullptr) {
            auto return_type = return_type_field->identifier()->type_ref()->type();
            auto target_number_class = return_type->number_class();
            auto target_size = return_type->size_in_bytes();
            auto target_type = target_number_class == type_number_class_t::integer ?
                               vm::register_type_t::integer :
                               vm::register_type_t::floating_point;

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

        return true;
    }

    bool procedure_call::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        auto identifier = _reference->identifier();
        if (identifier != nullptr) {
            auto proc_type = dynamic_cast<procedure_type*>(identifier->type_ref()->type());
            if (proc_type != nullptr) {
                if (proc_type->return_type() == nullptr)
                    return false;
                auto return_identifier = proc_type->return_type()->identifier();
                result.inferred_type = return_identifier->type_ref()->type();
                result.reference = return_identifier->type_ref();
                return true;
            }
        }
        return false;
    }

    compiler::argument_list* procedure_call::arguments() {
        return _arguments;
    }

    compiler::identifier_reference* procedure_call::reference(){
        return _reference;
    }

    void procedure_call::on_owned_elements(element_list_t& list) {
        if (_arguments != nullptr)
            list.emplace_back(_arguments);

        for (auto type_param : _type_parameters) {
            list.emplace_back(type_param);
        }
    }

    void procedure_call::reference(compiler::identifier_reference* value) {
        _reference = value;
    }

    const compiler::type_reference_list_t& procedure_call::type_parameters() const {
        return _type_parameters;
    }

};