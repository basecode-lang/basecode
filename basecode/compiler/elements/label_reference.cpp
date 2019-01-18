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
#include "label_reference.h"

namespace basecode::compiler {

    label_reference::label_reference(
        compiler::module* module,
        compiler::block* parent_scope,
        const std::string& label) : element(module, parent_scope, element_type_t::label_reference),
                                    _label(label) {
    }

    bool label_reference::on_fold(
            compiler::session& session,
            fold_result_t& result) {
        auto labels = session.elements().find_by_type<compiler::label>(element_type_t::label);
        for (auto label : labels) {
            if (label->name() == _label) {
                result.element = label;
                return true;
            }
        }
        return false;
    }

    std::string label_reference::label() const {
        return _label;
    }

    bool label_reference::on_is_constant() const {
        return true;
    }

};