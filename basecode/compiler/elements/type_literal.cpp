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
#include <vm/instruction_block.h>
#include "array_type.h"
#include "type_literal.h"
#include "type_reference.h"

namespace basecode::compiler {

    type_literal::type_literal(
            compiler::module* module,
            compiler::block* parent_scope,
            type_literal_type_t type,
            compiler::argument_list* args,
            const compiler::type_reference_list_t& type_params,
            const compiler::element_list_t& subscripts) : element(module, parent_scope, element_type_t::type_literal),
                                                          _type(type),
                                                          _subscripts(subscripts),
                                                          _args(args),
                                                          _type_params(type_params) {
    }

    bool type_literal::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        block->comment("XXX: type literal", 4);
        block->nop();

        return true;
    }

    bool type_literal::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        if (_type_params.empty())
            return false;

        switch (_type) {
            case type_literal_type_t::map:
            case type_literal_type_t::user:
            case type_literal_type_t::tuple:
            case type_literal_type_t::array: {
                result.reference = _type_params[0];
                result.inferred_type = _type_params[0]->type();
                break;
            }
        }
        return true;
    }

    bool type_literal::on_is_constant() const {
        return true;
    }

    compiler::argument_list* type_literal::args() {
        return _args;
    }

    type_literal_type_t type_literal::type() const {
        return _type;
    }

    const compiler::element_list_t& type_literal::subscripts() const {
        return _subscripts;
    }

    const compiler::type_reference_list_t& type_literal::type_params() const {
        return _type_params;
    }

};