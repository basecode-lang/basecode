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
#include <compiler/element_builder.h>
#include "procedure_type.h"
#include "inline_directive.h"

namespace basecode::compiler {

    inline_directive::inline_directive(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* expression) : directive(module, parent_scope),
                                             _expression(expression) {
    }

    bool inline_directive::on_fold(
            compiler::session& session,
            fold_result_t& result) {
        result.element = _expression;
        return true;
    }

    bool inline_directive::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        return _expression->infer_type(session, result);
    }

    bool inline_directive::is_valid_data() const {
        return false;
    }

    compiler::element* inline_directive::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        auto expr = _expression != nullptr ?
                    _expression->clone<compiler::element>(session, new_scope) :
                    nullptr;
        auto directive = session
            .builder()
            .make_directive(new_scope, type(), location(), {expr});
        return directive;
    }

    bool inline_directive::on_is_constant() const {
        return true;
    }

    directive_type_t inline_directive::type() const {
        return directive_type_t::inline_e;
    }

    compiler::element* inline_directive::expression() {
        return _expression;
    }

    bool inline_directive::on_evaluate(compiler::session& session) {
        switch (_expression->element_type()) {
            case element_type_t::proc_type: {
                auto proc_type = dynamic_cast<compiler::procedure_type*>(_expression);
                if (proc_type == nullptr)
                    return false;
                proc_type->is_inline(true);
                break;
            }
            default: {
                break;
            }
        }

        return true;
    }

}