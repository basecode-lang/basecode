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
            block* parent_scope,
            argument_list* args) : intrinsic(module, parent_scope, args) {
    }

    bool range_intrinsic::on_fold(
            compiler::session& session,
            fold_result_t& result) {
        return true;
    }

    bool range_intrinsic::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        auto& builder = session.builder();
        auto& scope_manager = session.scope_manager();
        auto int_type = scope_manager.find_type(qualified_symbol_t {
            .name = "s32"
        });

        compiler::element_list_t subscripts {};
        auto array_type = builder.make_array_type(
            scope_manager.current_scope(),
            session.program().block(),
            builder.make_type_reference(
                scope_manager.current_scope(),
                int_type->symbol()->qualified_symbol(),
                int_type),
            subscripts);

        result.inferred_type = array_type;
        result.reference = builder.make_type_reference(
            scope_manager.current_scope(),
            array_type->symbol()->qualified_symbol(),
            array_type);

        return true;
    }

    bool range_intrinsic::can_fold() const {
        return true;
    }

    std::string range_intrinsic::name() const {
        return "range";
    }

};