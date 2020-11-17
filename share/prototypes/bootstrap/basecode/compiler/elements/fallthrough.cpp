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
#include "fallthrough.h"

namespace basecode::compiler {

    fallthrough::fallthrough(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::label* label) : element(module, parent_scope, element_type_t::fallthrough),
                                      _label(label) {
    }

    compiler::label* fallthrough::label() {
        return _label;
    }

    void fallthrough::on_owned_elements(element_list_t& list) {
        if (_label != nullptr)
            list.emplace_back(_label);
    }

};