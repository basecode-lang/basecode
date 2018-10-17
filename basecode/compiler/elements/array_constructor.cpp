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
#include "type_reference.h"
#include "array_constructor.h"

namespace basecode::compiler {

    array_constructor::array_constructor(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::type_reference* type_ref,
            compiler::argument_list* args,
            const compiler::element_list_t& subscripts) : element(module, parent_scope, element_type_t::array_constructor),
                                                          _subscripts(subscripts),
                                                          _args(args),
                                                          _type_ref(type_ref) {
    }

    bool array_constructor::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.reference = _type_ref;
        result.inferred_type = _type_ref->type();
        return true;
    }

    bool array_constructor::on_is_constant() const {
        return true;
    }

    compiler::argument_list* array_constructor::args() {
        return _args;
    }

    compiler::type_reference* array_constructor::type_ref() {
        return _type_ref;
    }

    bool array_constructor::on_emit(compiler::session& session) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        block->comment("XXX: array constructor", 4);
        block->nop();

        return true;
    }

};