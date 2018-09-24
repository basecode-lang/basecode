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
#include "array_constructor.h"

namespace basecode::compiler {

    array_constructor::array_constructor(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::argument_list* args,
            const compiler::element_list_t& subscripts) : element(module, parent_scope, element_type_t::array_constructor),
                                                          _subscripts(subscripts),
                                                          _args(args) {
    }

    bool array_constructor::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        auto& builder = session.builder();
        auto& scope_manager = session.scope_manager();

        qualified_symbol_t entry_type_name {.name = "u8"};
        auto entry_type = scope_manager.find_type(entry_type_name);

        qualified_symbol_t array_type_name {
            .name = array_type::name_for_array(entry_type, _subscripts)
        };
        result.inferred_type = scope_manager.find_type(array_type_name);

        // XXX: move this to the ast_evaluator::array_constructor
        if (result.inferred_type == nullptr) {
            result.inferred_type = builder.make_array_type(
                parent_scope(),
                builder.make_block(parent_scope(), element_type_t::block),
                entry_type,
                entry_type_name,
                _subscripts);
            result.reference = builder.make_type_reference(
                parent_scope(),
                entry_type_name,
                result.inferred_type);
        }

        return true;
    }

    bool array_constructor::on_is_constant() const {
        return true;
    }

    compiler::argument_list* array_constructor::args() {
        return _args;
    }

    bool array_constructor::on_emit(compiler::session& session) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        block->comment("XXX: array constructor", 4);
        block->nop();

        return true;
    }

};