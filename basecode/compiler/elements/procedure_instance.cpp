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

#include "block.h"
#include "procedure_instance.h"

namespace basecode::compiler {

    procedure_instance::procedure_instance(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::type* procedure_type,
            compiler::block* scope) : element(module, parent_scope, element_type_t::proc_instance),
                                      _scope(scope),
                                      _procedure_type(procedure_type) {
    }

    bool procedure_instance::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        return _scope->emit(session, context, result);
    }

    compiler::block* procedure_instance::scope() {
        return _scope;
    }

    compiler::type* procedure_instance::procedure_type() {
        return _procedure_type;
    }

    void procedure_instance::on_owned_elements(element_list_t& list) {
        if (_scope != nullptr)
            list.emplace_back(_scope);
    }

};