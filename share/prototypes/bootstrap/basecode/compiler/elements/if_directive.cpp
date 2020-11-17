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
#include "if_directive.h"

namespace basecode::compiler {

    if_directive::if_directive(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* lhs,
            compiler::element* rhs,
            compiler::element* body) : directive(module, parent_scope, "if"),
                                       _lhs(lhs),
                                       _rhs(rhs),
                                       _body(body) {
    }

    bool if_directive::on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) {
        _lhs = fold_result.element;
        return true;
    }

    compiler::element* if_directive::true_body() const {
        return _true_body;
    }

    bool if_directive::on_evaluate(compiler::session& session) {
        auto value = false;
        auto next_branch = this;
        while (!value && next_branch != nullptr) {
            if (next_branch->_rhs == nullptr) {
                _true_body = next_branch->_lhs;
                break;
            }
            if (!next_branch->_lhs->as_bool(value)) {
                session.error(
                    module(),
                    "X000",
                    "#if/#elif requires constant boolean expression.",
                    location());
                return false;
            }
            if (value) {
                _true_body = next_branch->_body;
                break;
            }
            next_branch = dynamic_cast<compiler::if_directive*>(next_branch->_rhs);
        }
        return true;
    }

    void if_directive::on_owned_elements(element_list_t& list) {
        list.emplace_back(_lhs);
        list.emplace_back(_rhs);
        list.emplace_back(_body);
    }

};