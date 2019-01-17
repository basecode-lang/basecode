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
#include "type.h"
#include "cast.h"
#include "numeric_type.h"
#include "symbol_element.h"
#include "type_reference.h"

namespace basecode::compiler {

    cast::cast(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::type_reference* type,
            compiler::element* expr) : element(module, parent_scope, element_type_t::cast),
                                       _expression(expr),
                                       _type_ref(type) {
    }

    bool cast::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.inferred_type = _type_ref->type();
        result.reference = _type_ref;
        return true;
    }

    bool cast::on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) {
        _expression = fold_result.element;
        return true;
    }

    compiler::element* cast::expression() {
        return _expression;
    }

    compiler::type_reference* cast::type() {
        return _type_ref;
    }

    void cast::expression(compiler::element* value) {
        _expression = value;
    }

    void cast::on_owned_elements(element_list_t& list) {
        if (_expression != nullptr)
            list.emplace_back(_expression);
    }

    const common::source_location& cast::type_location() const {
        return _type_location;
    }

    void cast::type_location(const common::source_location& loc) {
        _type_location = loc;
    }

};