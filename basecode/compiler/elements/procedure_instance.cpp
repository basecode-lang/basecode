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
#include "block.h"
#include "procedure_type.h"
#include "procedure_instance.h"

namespace basecode::compiler {

    procedure_instance::procedure_instance(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::procedure_type* procedure_type,
            compiler::block* scope) : element(module, parent_scope, element_type_t::proc_instance),
                                      _scope(scope),
                                      _procedure_type(procedure_type) {
    }

    bool procedure_instance::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.types.emplace_back(_procedure_type);
        return true;
    }

    void procedure_instance::mark_as_template() {
        _is_template = true;
    }

    bool procedure_instance::is_template() const {
        return _is_template;
    }

    compiler::block* procedure_instance::scope() {
        return _scope;
    }

    compiler::procedure_type* procedure_instance::procedure_type() {
        return _procedure_type;
    }

    compiler::procedure_instance* procedure_instance::bake_for_types(
        compiler::session& session,
        const type_map_t& types) {
        return nullptr;
    }

    void procedure_instance::on_owned_elements(element_list_t& list) {
        if (_scope != nullptr)
            list.emplace_back(_scope);
    }

}