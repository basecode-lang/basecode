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
#include "type.h"
#include "transmute.h"
#include "symbol_element.h"
#include "type_reference.h"

namespace basecode::compiler {

    transmute::transmute(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::type_reference* type,
            compiler::element* expr) : element(module, parent_scope, element_type_t::transmute),
                                       _expression(expr),
                                       _type_ref(type) {
    }

    bool transmute::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.types.emplace_back(_type_ref->type(), _type_ref);
        return true;
    }

    bool transmute::on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) {
        _expression = fold_result.element;
        return true;
    }

    compiler::element* transmute::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        return session.builder().make_transmute(
            new_scope,
            _type_ref->clone<compiler::type_reference>(session, new_scope),
            _expression->clone<compiler::element>(session, new_scope));
    }

    compiler::element* transmute::expression() {
        return _expression;
    }

    compiler::type_reference* transmute::type() {
        return _type_ref;
    }

    void transmute::expression(compiler::element* value) {
        _expression = value;
    }

    void transmute::on_owned_elements(element_list_t& list) {
        if (_expression != nullptr)
            list.emplace_back(_expression);
    }

    const common::source_location& transmute::type_location() const {
        return _type_location;
    }

    void transmute::type_location(const common::source_location& loc) {
        _type_location = loc;
    }

}