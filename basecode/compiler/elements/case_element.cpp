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
#include "label.h"
#include "block.h"
#include "statement.h"
#include "case_element.h"
#include "binary_operator.h"

namespace basecode::compiler {

    case_element::case_element(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::block* scope,
            compiler::element* expr) : element(module, parent_scope, element_type_t::case_e),
                                       _scope(scope),
                                       _expr(expr) {
    }

    compiler::block* case_element::scope() {
        return _scope;
    }

    bool case_element::on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) {
        _expr = fold_result.element;
        return true;
    }

    compiler::element* case_element::expression() {
        return _expr;
    }

    void case_element::on_owned_elements(element_list_t& list) {
        if (_expr != nullptr)
            list.emplace_back(_expr);

        if (_scope != nullptr)
            list.emplace_back(_scope);
    }

};