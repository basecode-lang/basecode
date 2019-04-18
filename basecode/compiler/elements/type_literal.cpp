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
#include <compiler/element_builder.h>
#include "array_type.h"
#include "type_literal.h"
#include "argument_list.h"
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
        result.types.emplace_back(_type_ref->type(), _type_ref);
        return true;
    }

    compiler::element* type_literal::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        if (_type_ref->type()->is_array_type()) {
            return session.builder().make_array_literal(
                new_scope,
                _type_ref->clone<compiler::type_reference>(session, new_scope),
                compiler::clone(session, new_scope, _type_params),
                _args->clone<compiler::argument_list>(session, new_scope),
                compiler::clone(session, new_scope, _subscripts));
        } else {
            return session.builder().make_user_literal(
                new_scope,
                _type_ref->clone<compiler::type_reference>(session, new_scope),
                compiler::clone(session, new_scope, _type_params),
                _args->clone<compiler::argument_list>(session, new_scope));
        }
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