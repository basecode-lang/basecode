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
        compiler::argument_list* args) : element(module, parent_scope, element_type_t::proc_call),
                                         _arguments(args),
                                         _reference(reference) {
    }

    // XXX: not handling multiple returns yet
    bool procedure_call::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        auto identifier = _reference->identifier();
        if (identifier != nullptr) {
            auto proc_type = dynamic_cast<procedure_type*>(identifier->type_ref()->type());
            auto return_identifier = proc_type->return_type()->identifier();
            result.inferred_type = return_identifier->type_ref()->type();
            result.reference = return_identifier->type_ref();
            return true;
        }
        return false;
    }

    bool procedure_call::on_emit(compiler::session& session) {
        auto& assembler = session.assembler();

        auto block = assembler.current_block();
        auto identifier = _reference->identifier();
        auto init = identifier->initializer();
        if (init == nullptr)
            return false;

        auto procedure_type = init->procedure_type();

        if (_arguments != nullptr)
            _arguments->emit(session);

        if (procedure_type->is_foreign()) {
            block->comment(
                fmt::format("foreign call: {}", identifier->symbol()->name()),
                4);
            block->push_u16(static_cast<uint16_t>(_arguments->elements().size()));
            block->call_foreign(procedure_type->foreign_address());
        } else {
            block->call(assembler.make_label_ref(identifier->symbol()->name()));
        }

        auto target_reg = assembler.current_target_register();
        if (target_reg != nullptr) {
            if (procedure_type->return_type() != nullptr) {
                block->pop(*target_reg);
            }
        }

        return true;
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
    }

    void procedure_call::reference(compiler::identifier_reference* value) {
        _reference = value;
    }

};