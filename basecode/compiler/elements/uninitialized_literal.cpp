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
#include "uninitialized_literal.h"

namespace basecode::compiler {

    uninitialized_literal::uninitialized_literal(
            compiler::module* module,
            compiler::block* parent_scope) : compiler::element(module,
                                                               parent_scope,
                                                               element_type_t::uninitialized_literal) {
    }

    bool uninitialized_literal::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        return false;
    }

    bool uninitialized_literal::is_singleton() const {
        return true;
    }

    bool uninitialized_literal::on_is_constant() const {
        return true;
    }

}