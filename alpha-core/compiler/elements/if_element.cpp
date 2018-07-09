// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#include "if_element.h"

namespace basecode::compiler {

    if_element::if_element(
            element* parent,
            element* predicate,
            element* true_branch,
            element* false_branch) : element(parent, element_type_t::if_e),
                                     _predicate(predicate),
                                     _true_branch(true_branch),
                                     _false_branch(false_branch) {
    }

    bool if_element::on_emit(
            common::result& r,
            vm::assembler& assembler,
            const emit_context_t& context) {
        _predicate->emit(r, assembler, context);
        return true;
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

};