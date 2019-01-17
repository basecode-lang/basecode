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
#include "label.h"
#include "break_element.h"

namespace basecode::compiler {

    break_element::break_element(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* label) : element(module, parent_scope, element_type_t::break_e),
                                        _label(label) {
    }

    bool break_element::on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) {
        _label = fold_result.element;
        return true;
    }

    compiler::element* break_element::label() {
        return _label;
    }

    void break_element::on_owned_elements(element_list_t& list) {
        if (_label != nullptr)
            list.emplace_back(_label);
    }

};