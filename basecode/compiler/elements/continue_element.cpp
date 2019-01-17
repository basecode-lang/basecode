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
#include "continue_element.h"

namespace basecode::compiler {

    continue_element::continue_element(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* label) : element(module, parent_scope, element_type_t::continue_e),
                                        _label(label) {
    }

    bool continue_element::on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) {
        _label = fold_result.element;
        return true;
    }

    compiler::element* continue_element::label() {
        return _label;
    }

    void continue_element::on_owned_elements(element_list_t& list) {
        if (_label != nullptr)
            list.emplace_back(_label);
    }

};