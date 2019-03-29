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
#include "array_type.h"
#include "type_literal.h"
#include "type_reference.h"

namespace basecode::compiler {

    type_literal::type_literal(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::type_reference* type_ref,
            compiler::argument_list* args,
            const compiler::type_reference_list_t& type_params,
            const compiler::element_list_t& subscripts) : element(module, parent_scope, element_type_t::type_literal),
                                                          _subscripts(subscripts),
                                                          _args(args),
                                                          _type_ref(type_ref),
                                                          _type_params(type_params) {
    }

    bool type_literal::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.reference = _type_ref;
        result.inferred_type = _type_ref->type();
        return true;
    }

    bool type_literal::on_is_constant() const {
        return true;
    }

    compiler::argument_list* type_literal::args() {
        return _args;
    }

    compiler::type_reference* type_literal::type_ref() const {
        return _type_ref;
    }

    const compiler::element_list_t& type_literal::subscripts() const {
        return _subscripts;
    }

    const compiler::type_reference_list_t& type_literal::type_params() const {
        return _type_params;
    }

}