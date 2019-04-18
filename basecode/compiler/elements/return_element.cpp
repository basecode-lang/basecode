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
#include "block.h"
#include "return_element.h"

namespace basecode::compiler {

    return_element::return_element(
        compiler::module* module,
        block* parent_scope) : element(module, parent_scope, element_type_t::return_e) {
    }

    bool return_element::on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) {
        for (size_t i = 0; i < _expressions.size(); i++) {
            if (_expressions[i] == e)
                _expressions[i] = fold_result.element;
        }
        return true;
    }

    field_map_t* return_element::parameters() {
        return _parameters;
    }

    compiler::element* return_element::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        auto copy = session.builder().make_return(new_scope);
        // XXX: fix this, not sure it's really correct
        copy->_parameters = _parameters;
        copy->_expressions = compiler::clone(session, new_scope, _expressions);
        return copy;
    }

    element_list_t& return_element::expressions() {
        return _expressions;
    }

    void return_element::parameters(field_map_t* value) {
        _parameters = value;
    }

    void return_element::on_owned_elements(element_list_t& list) {
        for (auto element : _expressions)
            list.emplace_back(element);
    }

}