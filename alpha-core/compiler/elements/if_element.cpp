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
#include "if_element.h"

namespace basecode::compiler {

    if_element::if_element(
            compiler::module* module,
            block* parent_scope,
            element* predicate,
            element* true_branch,
            element* false_branch) : element(module, parent_scope, element_type_t::if_e),
                                     _predicate(predicate),
                                     _true_branch(true_branch),
                                     _false_branch(false_branch) {
    }

    element* if_element::predicate() {
        return _predicate;
    }

    element* if_element::true_branch() {
        return _true_branch;
    }

    element* if_element::false_branch() {
        return _false_branch;
    }

    bool if_element::on_emit(compiler::session& session) {
        return true;
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