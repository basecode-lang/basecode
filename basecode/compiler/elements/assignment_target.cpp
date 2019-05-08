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

#include <utility>
#include <compiler/session.h>
#include <compiler/element_builder.h>
#include "type.h"
#include "identifier.h"
#include "type_reference.h"
#include "assignment_target.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    assignment_target::assignment_target(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::identifier_reference_list_t refs) : element(module,
                                                                  parent_scope,
                                                                  element_type_t::assignment_target),
                                                          _refs(std::move(refs)) {
    }

    bool assignment_target::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        for (auto ref : _refs) {
            auto type_ref = ref->identifier()->type_ref();
            result.types.emplace_back(type_ref->type(), type_ref);
        }
        return true;
    }

    compiler::element* assignment_target::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        return session.builder().make_assignment_target(
            new_scope,
            _refs);
    }

    const compiler::identifier_reference_list_t& assignment_target::refs() const {
        return _refs;
    }

}