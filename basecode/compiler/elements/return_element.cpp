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

    element_list_t& return_element::expressions() {
        return _expressions;
    }

    void return_element::on_owned_elements(element_list_t& list) {
        for (auto element : _expressions)
            list.emplace_back(element);
    }

};