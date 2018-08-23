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

#include "field.h"
#include "identifier.h"

namespace basecode::compiler {

    field::field(
            compiler::module* module,
            block* parent_scope,
            compiler::identifier* identifier): element(module, parent_scope, element_type_t::field),
                                               _identifier(identifier) {
    }

    bool field::on_infer_type(
            const compiler::session& session,
            type_inference_result_t& result) {
        if (_identifier == nullptr)
            return false;
        return _identifier->infer_type(session, result);
    }

    compiler::identifier* field::identifier() {
        return _identifier;
    }

    void field::on_owned_elements(element_list_t& list) {
        if (_identifier != nullptr)
            list.emplace_back(_identifier);
    }

};