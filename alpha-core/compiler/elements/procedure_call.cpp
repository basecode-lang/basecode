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

#include <vm/instruction_block.h>
#include "program.h"
#include "identifier.h"
#include "argument_list.h"
#include "symbol_element.h"
#include "procedure_type.h"
#include "procedure_call.h"

namespace basecode::compiler {

    procedure_call::procedure_call(
        compiler::block* parent_scope,
        compiler::identifier* identifier,
        compiler::argument_list* args) : element(parent_scope, element_type_t::proc_call),
                                         _arguments(args),
                                         _identifier(identifier) {
    }

    bool procedure_call::on_emit(
            common::result& r,
            emit_context_t& context) {
        auto instruction_block = context.assembler->current_block();
        auto init = identifier()->initializer();
        if (init == nullptr)
            return false;

        if (_arguments != nullptr)
            _arguments->emit(r, context);

        auto procedure_type = init->procedure_type();
        if (procedure_type->is_foreign()) {
            instruction_block->call_foreign(identifier()->symbol()->name());
        } else {
            instruction_block->call(identifier()->symbol()->name());
        }

        auto target_reg = instruction_block->current_target_register();
        if (target_reg != nullptr) {
            if (!procedure_type->returns().as_list().empty())
                instruction_block->pop_u64(target_reg->reg.i);
        }

        return true;
    }

    compiler::identifier* procedure_call::identifier() {
        return _identifier;
    }

    compiler::argument_list* procedure_call::arguments() {
        return _arguments;
    }

    // XXX: not handling multiple returns yet
    compiler::type* procedure_call::on_infer_type(const compiler::program* program) {
        auto proc_type = dynamic_cast<procedure_type*>(_identifier->type());
        auto returns_list = proc_type->returns().as_list();
        return returns_list.front()->identifier()->type();
    }

};