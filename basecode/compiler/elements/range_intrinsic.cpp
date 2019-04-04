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
#include <compiler/scope_manager.h>
#include "type.h"
#include "array_type.h"
#include "argument_list.h"
#include "symbol_element.h"
#include "type_reference.h"
#include "range_intrinsic.h"

namespace basecode::compiler {

    range_intrinsic::range_intrinsic(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::argument_list* args,
            compiler::procedure_type* proc_type,
            const compiler::type_reference_list_t& type_params) : intrinsic(module,
                                                                            parent_scope,
                                                                            args,
                                                                            proc_type,
                                                                            type_params) {
    }

    bool range_intrinsic::on_fold(
            compiler::session& session,
            fold_result_t& result) {
        return true;
    }

    bool range_intrinsic::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        if (type_parameters().empty())
            return false;

        auto type_ref = type_parameters().front();
        result.types.emplace_back(type_ref->type(), type_ref);

        return true;
    }

    bool range_intrinsic::can_fold() const {
        return true;
    }

    intrinsic_type_t range_intrinsic::type() const {
        return intrinsic_type_t::range;
    }

}