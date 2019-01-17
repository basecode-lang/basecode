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

#include <fmt/format.h>
#include <compiler/session.h>
#include <vm/instruction_block.h>
#include "if_element.h"

namespace basecode::compiler {

    if_element::if_element(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* predicate,
            compiler::element* true_branch,
            compiler::element* false_branch,
            bool is_else_if) : element(module, parent_scope, element_type_t::if_e),
                               _is_else_if(is_else_if),
                               _predicate(predicate),
                               _true_branch(true_branch),
                               _false_branch(false_branch) {
    }

    bool if_element::is_else_if() const {
        return _is_else_if;
    }

    bool if_element::on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) {
        _predicate = fold_result.element;
        return true;
    }

    compiler::element* if_element::predicate() {
        return _predicate;
    }

    compiler::element* if_element::true_branch() {
        return _true_branch;
    }

    compiler::element* if_element::false_branch() {
        return _false_branch;
    }

    void if_element::predicate(compiler::element* value) {
        _predicate = value;
    }

    void if_element::on_owned_elements(element_list_t& list) {
        if (_predicate != nullptr)
            list.emplace_back(_predicate);
        if (_true_branch != nullptr)
            list.emplace_back(_true_branch);
        if (_false_branch != nullptr)
            list.emplace_back(_false_branch);
    }

};